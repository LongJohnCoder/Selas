
includedirs { MiddlewareDir .. "assimp-4.1.0\\include" }
includedirs { MiddlewareDir .. "rapidjson\\include" }
includedirs { MiddlewareDir .. "DirextXTex\\" }

table.insert(middlewareLinkDirectories, "DirectXTex\\Bin\\Desktop_2017\\x64\\Release")
table.insert(middlewareLinkDirectories, "assimp-4.1.0\\lib\\Release")
table.insert(middlewareLibraries, "assimp-vc140-mt")
table.insert(middlewareLibraries, "DirectxTex")
table.insert(postBuildCopies, "assimp-4.1.0\\bin\\Release\\assimp-vc140-mt.dll $(TargetDir)assimp-vc140-mt.dll")