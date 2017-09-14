-- flush les modules, si ils sont deja loades... --
--
package.loaded["Media"]=nil
package.loaded["Movie"]=nil
package.loaded["Image"]=nil
package.loaded["Model"]=nil
package.loaded["Capture"]=nil
package.loaded["Crayon"]=nil

require("Media")
require("Movie")
require("Image")
require("Model")
require("Capture")
require("Crayon")

--
-- patch pour le controleur midi
--
-- la table control contient ce qu'il faut faire avec les midi
-- controls[81] devrait contenir un objet obj= et param=
--
controls={}

function midiin (v,c)
	print("LUA midiin c="..c.." v="..v)
	OutReset()

	if controls[c] then
		ob=controls[c].obj
		if ob then
			f=ob[controls[c].param]
			if f then
				f(ob,v,time)
			else
				print("param ",controls[c].param," not assigned");
			end
		end
	end

	OutDump()
	return unpack(out)
end





--
-- Pour la gestion des messages... facile...
-- On a une fonction globale pour ajouter un message
-- et vider la queue de message
--
out={}
function OutReset() out={} end

function Out(...)
	for k,v in pairs({...}) do
		s,res=pcall(table.concat,v,",")
		if s then
			--print("inserting",res)
			table.insert(out,1,v)
		else
			--print("inserting (skip)")
		end
	end
end

function OutDump()
	for k,v in pairs(out) do
		print("message ",k,"is",table.concat(v,","))
	end
end

--
-- liste des  objets de type Media et autres...
-- notons que les objets sont "aware" que cette liste existe...
--

medias=
{
------------------------------
--- Masques (layer 0)  -------
------------------------------

----------------------
--- Yukie - Furie ----
----------------------
	{
	class=Movie,
	file="yukie/YukiFuri_City.mov",
	name="ecran",
	layer=1,
	fps=30,
	fade=0,
	assign={Fps=1001,Load=1073,Play=1065,Fade=1081}
	};
	{
	class=Movie,
	file="yukie/YukiFuri_Plage.mov",
	name="ecran",
	layer=2,
	fps=30,
	fade=0,
	assign={Fps=1002,Load=1074,Play=1066,Fade=1082}
	};
	{
	class=Movie,
	file="yukie/YukiFuri_Village.mov",
	name="ecran",
	layer=3,
	fps=30,
	fade=0,
	assign={Fps=1003,Play=1067,Load=1075,Fade=1083}
	};
	{
	class=Movie,
	file="yukie/ScanVertical.mov",
	name="ecran",
	layer=4,
	fps=30,
	fade=0,
	assign={Fps=1004,Play=1068,Load=1076,Fade=1084}
	};
	{
	class=Image,
	file="yukie/YukiFurie_alpha.png",
	name="ecran",
	layer=5,
	fade=0,
	assign={Load=1077,Fade=1085}
	};
	{
	class=Image,
	file="yukie/YukiFurie_alpha.png",
	name="ecran",
	layer=6,
	fade=0,
	assign={Load=1078,Fade=1086}
	};
	{
	class=Capture,
	camid=12146,
	name="ecran",
	layer=7,
	fade=0,
	scan=0,inverse=0,canny=0,
	tx=0,ty=0.75,sx=1,sy=-0.5,
	assign={Load=1079,Fade=1087,Canny=1039,Inverse=1047,Scanner=1055}
	};
	{
	class=Capture,
	camid=13137,
	name="ecran",
	layer=8,
	fade=0,
	scan=0,inverse=0,canny=0,
	tx=0,ty=0.75,sx=1,sy=-0.5,
	assign={Load=1080,Fade=1088,Canny=1040,Inverse=1048,Scanner=1056}
	};

-------------------------
--- Yukie - Endormie ----
-------------------------
	{
	class=Movie,
	file="yukie/YukiEndormiCity.mov",
	name="ecran",
	layer=1,
	fps=30,
	fade=0,
	assign={Fps=2001,Load=2073,Play=2065,Fade=2081}
	};
	{
	class=Movie,
	file="yukie/YukiEndormiPlage.mov",
	name="ecran",
	layer=2,
	fps=30,
	fade=0,
	assign={Fps=2002,Load=2074,Play=2066,Fade=2082}
	};
	{
	class=Movie,
	file="yukie/YukiEndormiVillage.mov",
	name="ecran",
	layer=3,
	fps=30,
	fade=0,
	assign={Fps=2003,Play=2067,Load=2075,Fade=2083}
	};
	{
	class=Movie,
	file="yukie/ScanHorizontal.mov",
	name="ecran",
	layer=4,
	fps=30,
	fade=0,
	assign={Fps=2002,Play=2068,Load=2076,Fade=2084}
	};
	{
	class=Image,
	file="yukie/YukiEndormi_alpha.png",
	name="ecran",
	layer=5,
	fade=0,
	assign={Load=2077,Fade=2085}
	};
	{
	class=Image,
	file="yukie/YukiEndormi_alpha.png",
	name="ecran",
	layer=6,
	fade=0,
	assign={Load=2078,Fade=2086}
	};
	{
	class=Capture,
	camid=12146,
	name="ecran",
	layer=7,
	fade=0,
	scan=0,inverse=0,canny=0,
	tx=0,ty=0.75,sx=1,sy=-0.5,
	assign={Load=2079,Fade=2087,Canny=2039,Inverse=2047,Scanner=2055}
	};
	{
	class=Capture,
	camid=13137,
	name="ecran",
	layer=8,
	fade=0,
	scan=0,inverse=0,canny=0,
	tx=0,ty=0.75,sx=1,sy=-0.5,
	assign={Load=2080,Fade=2088,Canny=2040,Inverse=2048,Scanner=2056}
	};


-------------------------
--- Yukie - Artist ------
-------------------------
	{
	class=Movie,
	file="yukie/YukiArtistCity.mov",
	name="ecran",
	layer=1,
	fps=30,
	fade=0,
	assign={Fps=3001,Load=3073,Play=3065,Fade=3081}
	};
	{
	class=Movie,
	file="yukie/YukiArtistPlage.mov",
	name="ecran",
	layer=2,
	fps=30,
	fade=0,
	assign={Fps=3002,Load=3074,Play=3066,Fade=3082}
	};
	{
	class=Movie,
	file="yukie/YukiArtistVillage.mov",
	name="ecran",
	layer=3,
	fps=30,
	fade=0,
	assign={Fps=3003,Play=3067,Load=3075,Fade=3083}
	};
	{
	class=Movie,
	file="yukie/ScanHorizontal.mov",
	name="ecran",
	layer=4,
	fps=30,
	fade=0,
	assign={Fps=3004,Play=3068,Load=3076,Fade=3084}
	};
	{
	class=Image,
	file="yukie/YukiArtiste_alpha.png",
	name="ecran",
	layer=5,
	fade=0,
	assign={Load=3077,Fade=3085}
	};
	{
	class=Image,
	file="yukie/YukiArtiste_alpha.png",
	name="ecran",
	layer=6,
	fade=0,
	assign={Load=3078,Fade=3086}
	};
	{
	class=Capture,
	camid=12146,
	name="ecran",
	layer=7,
	fade=0,
	scan=0,inverse=0,canny=0,
	tx=0,ty=0.75,sx=1,sy=-0.5,
	assign={Load=3079,Fade=3087,Canny=3039,Inverse=3047,Scanner=3055}
	};
	{
	class=Capture,
	camid=13137,
	name="ecran",
	layer=8,
	fade=0,
	scan=0,inverse=0,canny=0,
	tx=0,ty=0.75,sx=1,sy=-0.5,
	assign={Load=3080,Fade=3088,Canny=3040,Inverse=3048,Scanner=3056}
	};

}


--
-- pour faire un object, simplement associer la bonne metatable
-- on utilise le champs "class" d'une table pour savoir sa classe
--

for k,v in pairs(medias) do
	setClass(v)
end

--
-- init all media objects
--
--
OutReset()
for k,v in pairs(medias) do
	print("-- index ",k,"--")
	v:init()
	v:define_midi_controls()
	v:dump()
end

--
-- control midi table
--
for k,v in pairs(controls) do
	print("-- control ",k,":",v.obj," param ",v.param)
end


OutDump()

--
-- test
--
--midiin(100,81)
--midiin(10,1)

function refreshMidi ()
	print("--refreshing midi state--")
	for k,v in pairs(medias) do
		v:midiState()
	end
	return unpack(out)
end

refreshMidi()

--
-- on retourne les messages depart... surtout une mise a jour des parametres midi
--

return unpack(out)
