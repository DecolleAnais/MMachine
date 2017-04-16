mmachine_files = { gkit_dir .. "/MMachine/src/*.cpp", gkit_dir .. "/MMachine/src/*.hpp" }

mmachine_steps = {
  "mm_player",
  "mm_terrain"
}

for i, name in ipairs(mmachine_steps) do
  project(name)
  	language "C++"
  	kind "ConsoleApp"
  	targetdir "bin"
  	files ( gkit_files )
  	files ( mmachine_files )
  	files { gkit_dir .. "/MMachine/" .. name .. ".cpp" }
end



