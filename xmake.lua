add_rules("mode.debug", "mode.release")

set_languages("c++23")
set_optimize("none")

add_requires("libsdl3")
add_requires("libsdl3_image")
add_requires("glm")

target("shaders")
    set_kind("phony")
    before_build(function ()
        -- Récupérer tous les shaders dans le dossier "shaders"
        local vertShaders = os.files("shaders/*.vert")
        local fragShaders = os.files("shaders/*.frag")
        local shader_files = table.join(vertShaders, fragShaders)

        -- Créer le dossier des shaders compilés
        local compiled_dir = "compiled_shaders"
        os.mkdir(compiled_dir)

        -- Compiler chaque shader en SPIR-V
        for _, shader in ipairs(shader_files) do
            local output = path.join(compiled_dir, path.filename(shader) .. ".spv")
            print("Compilation de " .. shader .. " -> " .. output)
            os.run("glslc %s -o %s", shader, output)
        end
    end)

target("PixL-Engine")
    set_kind("binary")
    add_includedirs("include")
    add_files("src/**.cpp")
    add_packages("libsdl3", "libsdl3_image", "glm")

    -- Assurer que les shaders sont compilés avant l'application
    add_deps("shaders")

    -- Copier les shaders compilés dans le dossier de l'exécutable après compilation
    after_build(function (target)
        local exec_dir = target:targetdir() -- Récupérer le dossier où est placé l'exécutable
        local compiled_dir = "compiled_shaders"

        -- Créer le dossier de destination si nécessaire
        os.mkdir(path.join(exec_dir, "shaders"))

        -- Copier tous les shaders compilés dans le dossier de l'exécutable
        os.cp(path.join(compiled_dir, "*"), path.join(exec_dir, "shaders"))

        print("Shaders compilés copiés dans : " .. path.join(exec_dir, "shaders"))
    end)
