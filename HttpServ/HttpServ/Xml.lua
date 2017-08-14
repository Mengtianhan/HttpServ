require("LuaXml")
Xml = LuaXml.New()
local metaT = getmetatable(Xml)
metaT.__index = metaT
metaT.WriteFile = LuaXml.WriteFile
metaT.ReadFile  = LuaXml.ReadFile
metaT.PutData   = LuaXml.PutData
metaT.PutTable  = LuaXml.PutTable
metaT.GetData   = LuaXml.GetData
metaT.GetTable  = LuaXml.GetTable
metaT.CloseFile = LuaXml.CloseFile


local function tobool(str)
	local __result = false
	if str == "true" then
		__result = true
	end
	return __result;
end


metaT.tobool    = tobool
