package("tinyply")

    set_homepage("https://github.com/ddiakopoulos/tinyply")
    set_description("C++11 ply 3d mesh format importer & exporter")
    set_license("BSD-2-Clause")

    add_urls("https://github.com/ddiakopoulos/tinyply/archive/$(version).tar.gz")
    add_urls("https://github.com/ddiakopoulos/tinyply.git")
    add_versions("2.3.4", "1bb1462727a363f7b77a10e51cd023095db7b281d2f201167620a83e495513c6")

    on_install("macosx", "linux", "windows", "mingw", "android", "iphoneos", function (package)
        io.writefile("xmake.lua", [[
            add_rules("mode.debug", "mode.release")
            target("tinyply")
                set_kind("$(kind)")
                add_files("source/tinyply.cpp")
                add_headerfiles("source/tinyply.h")
        ]])
        import("package.tools.xmake").install(package)
    end)

    on_test(function (package)
        assert(package:check_cxxsnippets({test = [[
            void test() {
                tinyply::PlyFile plyFile;
            }
        ]]}, {configs = {languages = "c++11"}, includes = "tinyply.h"}))
    end)

package_end()

set_project("simviewer")
set_languages("cxxlatest")

add_rules("mode.debug", "mode.release")

add_requires("cxxopts v3.1.1")
add_requires("eigen")
add_requires("glad")
add_requires("glfw")
add_requires("glm")
add_requires("imgui v1.89.6", { configs = { glfw_opengl3 = true } })
add_requires("nativefiledialog-extended v1.0.2")
add_requires("spdlog", { configs = { fmt_external = true } })
add_requires("tinyply")
add_requires("yaml-cpp 0.7.0")

target("assets")
    set_kind("phony")
    after_build(function (target)
        os.mkdir(path.join(target:targetdir(), "assets"))
        os.cp(os.scriptdir() .. "/assets/*", path.join(target:targetdir(), "assets"))
    end)
    after_install(function (target)
        os.mkdir(path.join(target:installdir(), "assets"))
        os.cp(os.scriptdir() .. "assets/*", path.join(target:installdir(), "assets"))
    end)
    after_clean(function (target)
        os.rm(path.join(target:targetdir(), "assets"))
    end)

target("viewer")
    set_kind("binary")
    add_deps("assets")
    add_packages("cxxopts")
    add_packages("eigen")
    add_packages("glad")
    add_packages("glfw")
    add_packages("glm")
    add_packages("imgui")
    add_packages("nativefiledialog-extended")
    add_packages("spdlog")
    add_packages("tinyply")
    add_packages("yaml-cpp")

    add_includedirs("engine")
    add_headerfiles("engine/**.h")
    add_files("engine/**.cpp")
    add_headerfiles("viewer/**.h")
    add_files("viewer/**.cpp")
