extern crate tini;

use std::io;
use std::path::Path;
use std::collections::HashSet;

#[derive(PartialEq, Eq, Hash)]
pub struct Layer {
	pub layer_name: String,
	pub file_name: String,
	pub volume: u32,
	pub delay: u32
}

impl Layer {
	pub fn new(layer_name: &String) -> Layer {
		Layer{layer_name: String::clone(layer_name), file_name: String::new(), volume: 100, delay: 0}
	}

	pub fn set_file_name(&mut self, file_name: String) {
		if !self.file_name.is_empty() {
			self.file_name = String::new();
		}

		self.file_name.push_str(file_name.as_str());
	}
}

pub fn collect_layers(file: &Path) -> io::Result<HashSet<Layer>> {
	let mut layer_set: HashSet<Layer> = HashSet::new();

	if !file.is_file() {
		return Err(io::Error::new(io::ErrorKind::InvalidInput, "{} is not a file."));
	}

	let ini = match tini::Ini::from_file(Path::new(file)) {
		Err(e) => return Err(e),
		Ok(i) => i
	};

	for (section, iter) in ini.iter() {
		let mut layer: Layer = Layer::new(section);
		
		for (key, value) in iter {
			match key.as_str() {
				"File" => layer.set_file_name(format!("{}.ogg", value)),
				"Start Volume" => {
					let numval = match value.parse::<u32>() {
						Err(e) => return Err(io::Error::new(io::ErrorKind::InvalidData, "Couldn't parse string as u32.")),
						Ok(num) => num
					};

					if numval > 100 {
						println!("Start Volume > 100");
					}

					layer.volume = numval;
				},
				"Delay" => {
					layer.delay = match value.parse::<u32>() {
						Err(e) => return Err(io::Error::new(io::ErrorKind::InvalidData, "Couldn't parse string as u32.")),
						Ok(num) => num
					};
				},
				_ => {}
			}
		}

		if layer.file_name.is_empty() {
			println!("Layer with no File {}.", section);
			continue;
		}
		
		layer_set.insert(layer);
	}
	
	Ok(layer_set)
}

pub fn find_layer_by_name<'a>(layer_set: &'a HashSet<Layer>, layer_name: &str) -> Option<&'a Layer> {
	for layer in layer_set {
		if layer.layer_name == layer_name {
			return Some(layer);
		}
	}
	
	None
}

