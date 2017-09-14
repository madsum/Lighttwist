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
			print("inserting",res)
			table.insert(out,1,v)
		else
			print("inserting (skip)")
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
	{
	class=Movie,
	file="test/plantspano2.mpg",
	layer=0,
	fps=30,
	fade=1,
	assign={Fps=1,Play=65,Load=73,Fade=81}
	};
	{
	class=Image,
	file="test/logo-udm.png",
	layer=1,
	fade=0,
	assign={Tx=2,Ty=10,Sx=18,Sy=26,Play=66,Load=74,Fade=82}
	};
	{
	class=Model,
	file="test/particulesNeige.osg",
	layer=2,
	fade=0,
	assign={Tx=3,Ty=11,Sx=19,Sy=27,Play=67,Load=75,Fade=83}
	};
	{
	class=Capture,
	tx=0,ty=0,sx=1,sy=1,
	camera="gige:13139",
	layer=4,
	fade=1,
	assign={Scanner=36,Tx=4,Ty=12,Sx=20,Sy=28,Play=68,Load=76,Fade=84}
	};
	--{
	--class=Capture,
	--sx=4,sy=1,tx=-0.40,ty=0,
	--camera="gige:13139",
	--layer=4,
	--fade=1,
	--assign={Scanner=34,Tx=5,Ty=13,Sx=21,Sy=29,Play=69,Load=77,Fade=85}
	--};
	{
	class=Model,
	file="test/test_scene.osg",
	layer=2,
	fade=0,
	assign={Scanner=34,Tx=5,Ty=13,Sx=21,Sy=29,Play=69,Load=77,Fade=85}
	};
	{
	class=Crayon,
	sx=1,sy=1,tx=0,ty=0,
        layer=2,
        fade=1,
	assign={Tx=6,Ty=14,Sx=22,Sy=30,Play=70,Load=78,Fade=86}
	};


-------------------
-- Je Ne ----------
-------------------
	{
	class=Movie,
	file="flous/01_JeNe01_NuageNoir.avi",
	layer=0,
	fps=30,
	fade=1,
	assign={Fps=1001,Play=1065,Load=1073,Fade=1081}
	};
	{
	class=Movie,
	file="flous/01_JeNe02_NuageNoir.avi",
	layer=1,
	fps=30,
	fade=1,
	assign={Fps=1002,Play=1066,Load=1074,Fade=1082}
	};
	{
	class=Movie,
	file="flous/01_JeNe03_ArbresNoirs.avi",
	layer=2,
	fps=30,
	fade=1,
	assign={Fps=1003,Play=1067,Load=1075,Fade=1083}
	};
	{
	class=Movie,
	file="flous/01_JeNe04_ArbresVerts.avi",
	layer=3,
	fps=30,
	fade=1,
	assign={Fps=1004,Play=1068,Load=1076,Fade=1084}
	};
	{
	class=Movie,
	file="flous/01_JeNe05_ArbresTexture.avi",
	layer=4,
	fps=30,
	fade=1,
	assign={Fps=1005,Play=1069,Load=1077,Fade=1085}
	};
	{
	class=Capture,
	tx=0,ty=0,sx=1,sy=1,
	camera="gige:13139",
	layer=4,
	fade=1,
	assign={Scanner=1038,Tx=6,Ty=14,Sx=22,Sy=28,Freeze=1070,Load=1078,Fade=1086}
	};

-------------------
-- Sommeil Rouge --
-------------------
	{
	class=Movie,
	file="flous/02_Rouge01_Ligne.avi",
	layer=0,
	fps=30,
	fade=1,
	assign={Fps=2001,Play=2065,Load=2073,Fade=2081}
	};
	{
	class=Movie,
	file="flous/02_Rouge02_Vibration.avi",
	layer=1,
	fps=30,
	fade=1,
	assign={Fps=2002,Play=2066,Load=2074,Fade=2082}
	};
	{
	class=Movie,
	file="flous/02_Rouge03_Texture.avi",
	layer=2,
	fps=30,
	fade=1,
	assign={Fps=2003,Play=2067,Load=2075,Fade=2083}
	};

-------------------
-- Reneiges -------
-------------------
    {
	class=Image,
	file="flous/lune_blanche.png",
	layer=0,
	fade=1,
	assign={Fps=3001,Play=3065,Load=3073,Fade=3081}
	};
	{
	class=Movie,
	file="flous/jeu_sejour_ok.avi",
	layer=1,
	fps=30,
	fade=1,
	assign={Fps=3002,Play=3066,Load=3074,Fade=3082}
	--Fps=3002,
    };
	{
	class=Movie,
	file="flous/pointe_video1.avi",
	layer=2,
	fps=30,
	fade=1,
	assign={Fps=3003,Play=3067,Load=3075,Fade=3083}
	};
	{
	class=Movie,
	file="flous/neige.avi",
	layer=3,
	fps=15,
	fade=1,
	assign={Fps=3036,Play=3068,Load=3076,Fade=3084}
	};
	{
	class=Crayon,
    --	-0.140625	-1	-0.421875	0.984375
	--  0.187500 -1.000000 0.406250 0.984375
    sx=-0.140625,sy=-1,tx=-0.421875,ty=0.984375,
        layer=4,
        fade=1,
	assign={Load=3077,Fade=3085}
	};
	{
	class=Capture,
	tx=0,ty=0,sx=1,sy=1,
	camera="gige:13139",
	layer=0,
	fade=1,
	assign={Scanner=3038,Tx=3006,Ty=3014,Sx=3022,Sy=3030,Freeze=3070,Load=3078,Fade=3086}
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
	--v:dump()
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
