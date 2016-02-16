auto Icarus::wonderSwanManifest(string location) -> string {
  vector<uint8_t> buffer;
  concatenate(buffer, {location, "program.rom"});
  return wonderSwanManifest(buffer, location);
}

auto Icarus::wonderSwanManifest(vector<uint8_t>& buffer, string location) -> string {
  string manifest;
  string digest = Hash::SHA256(buffer.data(), buffer.size()).digest();

  if(settings["icarus/UseDatabase"].boolean() && !manifest) {
    for(auto node : database.wonderSwan) {
      if(node["sha256"].text() == digest) {
        manifest.append(node.text(), "\n  sha256:   ", digest, "\n");
        break;
      }
    }
  }

  if(settings["icarus/UseHeuristics"].boolean() && !manifest) {
    WonderSwanCartridge cartridge{buffer.data(), buffer.size()};
    if(manifest = cartridge.manifest) {
      manifest.append("\n");
      manifest.append("information\n");
      manifest.append("  title:  ", prefixname(location), "\n");
      manifest.append("  sha256: ", digest, "\n");
      manifest.append("  note:   ", "heuristically generated by icarus\n");
    }
  }

  return manifest;
}

auto Icarus::wonderSwanImport(vector<uint8_t>& buffer, string location) -> string {
  auto name = prefixname(location);
  auto source = pathname(location);
  string target{settings["Library/Location"].text(), "WonderSwan/", name, ".ws/"};
//if(directory::exists(target)) return failure("game already exists");

  auto manifest = wonderSwanManifest(buffer, location);
  if(!manifest) return failure("failed to parse ROM image");

  if(!directory::create(target)) return failure("library path unwritable");
  if(file::exists({source, name, ".sav"}) && !file::exists({target, "save.ram"})) {
    file::copy({source, name, ".sav"}, {target, "save.ram"});
  }

  if(settings["icarus/CreateManifests"].boolean()) file::write({target, "manifest.bml"}, manifest);
  file::write({target, "program.rom"}, buffer);
  return success(target);
}
