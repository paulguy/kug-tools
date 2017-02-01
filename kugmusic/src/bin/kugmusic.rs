extern crate kugmusic;
extern crate tini;

use std::io::{self, Write};
use std::fs;
use std::path::Path;
use std::collections::HashSet;
use std::process::exit;
use std::mem::drop;
use std::env;

use kugmusic::song::{self, Song};
use kugmusic::layer::{self, Layer};

fn get_files_by_suffix(dir: &Path, suffix: &str) -> io::Result<HashSet<String>> {
	let mut file_set: HashSet<String> = HashSet::new();

	if dir.is_dir() {
		for entry in try!(fs::read_dir(dir)) {
			let entry = try!(entry);
			if try!(entry.file_type()).is_file() {
				let file_name = match entry.file_name().into_string() {
					Err(e) => return Err(io::Error::new(io::ErrorKind::InvalidData, "Couldn't decode filename as UTF-8.")),
					Ok(s) => s
				};

				if file_name.ends_with(suffix) {
					file_set.insert(
						match entry.path().into_os_string().into_string() {
							Err(e) => return Err(io::Error::new(io::ErrorKind::InvalidData, "Couldn't decode path as UTF-8.")),
							Ok(s) => s
						}
					);
				}
			}
		}
	}

	file_set.shrink_to_fit();

	Ok(file_set)
}

fn get_settings_files(dir: &Path) -> io::Result<HashSet<String>> {
	get_files_by_suffix(dir, "Settings")
}

fn write_ffmpeg_script(music_set: &HashSet<Song>, filename: &Path) -> io::Result<()> {
	let mut out: fs::File = try!(fs::File::create(filename));

	for song in music_set.into_iter() {
		if song.song_layers.len() > 1 {
			try!(write!(out, "echo \"Converting song {}...\"\n", song.song_name));

			try!(write!(out, "ffmpeg "));

			for layer in &song.song_layers {
				if layer.delay != 0 {
					try!(write!(out, "-itsoffset {} ", layer.delay));
				}
				try!(write!(out, "-i \"{}\" ", layer.file_name));
			}

			try!(write!(out, "-filter_complex \""));

			let mut i :usize = 0;
			for layer in &song.song_layers {
				if layer.volume != 100 {
					// convert all to float at this point since amix takes float values anyway
					try!(write!(out, "[{}:a]volume={}/100:precision=float[vol{}],", i, layer.volume, i));
				}
				i+=1;
			}

			i = 0;
			for layer in &song.song_layers {
				if layer.volume != 100 {
					try!(write!(out, "[vol{}]", i));
				} else {
					try!(write!(out, "[{}:a]", i));
				}
				i+=1;
			}

			try!(write!(out, "amix=inputs={}\"", song.song_layers.len()));

			try!(write!(out, " \"{}.wav\"\n", song.song_name));
		} else {
			try!(write!(out, "echo \"Copying unlayered {}...\"\n", song.song_layers[0].file_name));
			try!(write!(out, "cp \"{}\" \"{}.ogg\"\n", song.song_layers[0].file_name, song.song_name));
		}
	}

	Ok(())
}

fn main() {
	let mut args: env::Args = env::args();
	args.next();

	let base_dir: String = match args.next() {
		Some(d) => d,
		None => {
			println!("USAGE: kugmusic <dir>");
			println!("dir must contain World/ with '* Settings' files as well as Music/Music.ini.");
			exit(1);
		}
	};

	let layer_set: HashSet<Layer>;
	let file_set: HashSet<String>;
	let music_set: HashSet<Song>;

	println!("Collecting layer names...");

	layer_set = match layer::collect_layers(Path::new(&format!("{}/Music/Music.ini", base_dir))) {
		Err(e) => {
			println!("Couldn't get layers: {}", e);
			exit(1);
		},
		Ok(ls) => ls
	};
/*
	for layer in &layer_set {
		println!("Layer name: {}\nFile name: {}\nVolume: {}\nDelay: {}\n", layer.layer_name, layer.file_name, layer.volume, layer.delay);
	}
*/
	println!("Collecting map settings files...");

	file_set = match get_settings_files(Path::new(&format!("{}/World", base_dir))) {
		Err(e) => {
			println!("Couldn't get files list: {}", e);
			exit(1);
		},
		Ok(fl) => fl
	};

	if file_set.is_empty() {
		println!("No files found!");
		exit(1);
	}

	println!("Collecting song data from settings files...");

	music_set = match song::collect_music(&file_set, &layer_set) {
		Err(e) => {
			println!("Couldn't get music: {}", e);
			exit(1);
		},
		Ok(ms) => {
			ms
		}
	};

	drop(file_set);
/*	
	for item in music_set {
		println!("Song: {}", item.song_name);
		for (i, item2) in item.song_layers.into_iter().enumerate() {
			println!("Layer {}: {}", i, item2);
		}
		println!();
	}
*/

	println!("Writing ffmpeg script...");

	match write_ffmpeg_script(&music_set, Path::new(&format!("{}/Music/extract.sh", base_dir))) {
		Err(e) => {
			println!("Couldn't get music: {}", e);
			exit(1);
		},
		Ok(()) => println!("Success")
	}

	exit(0);
}
