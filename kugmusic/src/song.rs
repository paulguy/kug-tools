extern crate tini;

use layer::{self, Layer};
use std::io;
use std::path::Path;
use std::collections::HashSet;
use std::iter::FromIterator;

#[derive(PartialEq, Eq, Hash)]
pub struct Song<'a> {
	pub song_name: String,
	pub song_layers: Vec<&'a Layer>
}

impl<'a> Song<'a> {
	pub fn new() -> Song<'a> {
		Song{song_name: String::new(), song_layers: Vec::new()}
	}

	pub fn set_name(&mut self, song_name: String) {
		if !self.song_name.is_empty() {
			self.song_name = String::new();
		}

		song_name.replace("/", "_");
		self.song_name.push_str(song_name.as_str());
	}

	pub fn invalidate(&mut self) { // song name is checked to determine if any entries were written so pretend that never happened
		self.song_name = String::new();
	}

	pub fn add_layer(&mut self, layer: &'a Layer) {
		self.song_layers.push(layer);
	}

	pub fn same(&self, other: &Song) -> bool {
		if self.song_layers.len() != self.song_layers.len() {
			return false;
		}

		let mut accounted_for: Vec<bool> = Vec::with_capacity(self.song_layers.len());
		for i in 0..self.song_layers.len() {
			accounted_for.push(false);
		}

		let mut i: usize = 0;
		for item in &self.song_layers {
			for item2 in &other.song_layers {
				if accounted_for[i] == false && item == item2 {
					accounted_for[i] = true;
					break;
				}
			}
			i+=1;
		}

		if accounted_for.contains(&false) {
			return false;
		}

		true
	}
}	

fn song_in_set(music_set: &HashSet<Song>, song2: &Song) -> bool {
	for song in music_set {
		if song.same(&song2) {
			return true;
		}
	}

	false
}

fn song_name_in_set(music_set: &HashSet<Song>, song_name: &str) -> bool {
	for song in music_set {
		if song.song_name == song_name {
			return true;
		}
	}

	false
}

pub fn collect_music<'a>(file_set: &HashSet<String>, layer_set: &'a HashSet<Layer>) -> io::Result<HashSet<Song<'a>>> {
	let mut music_set: HashSet<Song> = HashSet::new();

	for file_name in file_set {
		let ini = match tini::Ini::from_file(Path::new(file_name.as_str())) {
			Err(e) => return Err(e),
			Ok(i) => i
		};

		for (section, iter) in ini.iter() {
			if section == "General" {
				let mut song: Song = Song::new();

				for (key, value) in iter {
					if key.starts_with("Music ") && !key.ends_with(" Group") {
						if value.is_empty() { // some have empty Music entries for some reason
							continue;
						}

						if song.song_name.is_empty() {
							let mut song_name: String = String::clone(value);

							if song_name.starts_with("Oneshots/") {
								song.set_name(String::from_iter(song_name.drain(9..)));
							} else if let Some(i) = song_name.rfind("/") {
								if i == 1 { // unlikely but handled just in case
									song.set_name(String::from_iter(song_name.drain(1..)));
								} else {
									song.set_name(String::from_iter(song_name.drain(..i)));
								}
							} else {
								song.set_name(song_name);
							}
						}

						let layer: &Layer = match layer::find_layer_by_name(layer_set, value) {
							Some(l) => l,
							None => {
								println!("In {}", file_name);
								println!("Couldn't find layer {}", value);
								song.invalidate();
								break;
							}
						};

						song.add_layer(layer);
					}
				}

				// Screen with no music
				if song.song_name.is_empty() {
					break;
				}

				// Duplicate song
				if song_in_set(&music_set, &song) {
					break;
				}

				// Different song with same resolved name
				if song_name_in_set(&music_set, &song.song_name) {
					for i in 0.. {
						let temp_song_name: String;

						temp_song_name = format!("{} {}", song.song_name, i);
						if song_name_in_set(&music_set, &temp_song_name) {
							continue;
						}

						song.set_name(temp_song_name);
						break;
					}
				}

				music_set.insert(song);
			}
		}
	}

	Ok(music_set)
}

