
--
-- initialisation of a luabox
--
-- a pd message of the form  abc x y z 1 2
-- will call abc("x","y","z",1,2)
-- the return values are sent to the outlets
--
--

print("init.lua")

function toto (a,b)
	print("toto called with a="..a.." and b="..b)
	return a+b
end

function blub (a,b)
	print("blub called with a="..a.." and b="..b)
	return {"ret",a+1,b+1,a+b},a*a,11,22,33,44,b*b
end


function list (...)
	print("merci pour la liste! j'ai recu:",...)
	a={...}
	return a[2]
end

function float (v)
	print("merci pour le float "..v)
	return nil,nil,v+1
end

