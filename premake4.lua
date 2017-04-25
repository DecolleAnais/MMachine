mmachine_files = { gkit_dir .. "/MMachine/src/*.cpp", gkit_dir .. "/MMachine/src/*.hpp" }

mmachine_steps = {
  "mm_player",
--~  "mm_terrain"
}

for i, name in ipairs(mmachine_steps) do
  project(name)
<<<<<<< HEAD
    language "C++"
=======
  	language "C++"
>>>>>>> 226e73f44314572eba1dd8217a22fd0d71ae5b1c
  	kind "ConsoleApp"
  	targetdir "bin"
  	files ( gkit_files )
  	files ( mmachine_files )
  	files { gkit_dir .. "/MMachine/" .. name .. ".cpp" }
end



