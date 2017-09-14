require("Player")

--
-- Classe Media... contient les image, movie, model (3d),...
-- et tout ce qui peut etre affiche sur un layer (load/unload)
-- et un fade
--
-- layer
-- fade
-- assign
--
-- loaded	(intern state)
--

--
-- les classes sont definies ici au depart... vide...
--
Media={}
setClass(Media,Player)

function Media:init ()
	Player.init(self)

	self.loaded=false
	self.fade=0.0

        if not self.tx then self.tx=0 end
        if not self.ty then self.ty=0 end
        if not self.sx then self.sx=1 end
        if not self.sy then self.sy=1 end
end

function Media:dump ()
	Player.dump(self)
	print("Media: layer",self.layer)
	print("Media: fade",self.fade)
	print("Media: assign")
	for k,v in pairs(self.assign) do print("     ",k," midi ",v) end
	print("Media: loaded",self.loaded)

        print("Media: tx",self.tx)
        print("Media: ty",self.ty)
        print("Media: sx",self.sx)
        print("Media: sy",self.sy)

end

function Media:define_midi_controls ()
	for k,v in pairs(self.assign or {}) do
		controls[v]={obj=self,param=k}
	end
end


--
--
-- fonctions parametres (appele par controles midi) --
-- elles commencent par une majuscule...
-- elle sont appellees avec les parametres midi_value,time
--



function Media:Tx (v,time)
        self.tx=(v-64.0) / 12.0
        if self.loaded then
                --Out( {"net",self.name,"send","tx",self.layer,self.tx} )
                Out( {"net",self.name,"send","scaletrans",self.layer,self.sx, self.sy, self.tx, self.ty} )
        end
end

function Media:Ty (v,time)
        self.ty=(v-64.0) / 12.0
        if self.loaded then
                Out( {"net",self.name,"send","scaletrans",self.layer,self.sx, self.sy, self.tx, self.ty} )
        end
end

function Media:Sx (v,time)
        self.sx=(v-64.0) / 12.0
        if self.loaded then
                Out( {"net",self.name,"send","scaletrans",self.layer,self.sx, self.sy, self.tx, self.ty} )
        end
end

function Media:Sy (v,time)
        self.sy=(v-64.0) / 12.0
        if self.loaded then
                Out( {"net",self.name,"send","scaletrans",self.layer,self.sx, self.sy, self.tx, self.ty} )
        end
end



function Media:Load (v,time)
	if v<64 then
		-- on est plus actif.
		self.loaded=false
	else
		-- on devient actif
		-- passe tous les objets sur le meme layer (et meme player) et unload
		for k,ob in pairs(medias) do
			if not (ob==self) then
				if ob.loaded and ob.layer==self.layer and ob.name==self.name then
					ob:Play(0,time)
					ob.loaded=false
					ob:midiState()
				end
			end
		end
		self.loaded=true
    -- Out( {"net",self.name,"send","scaletrans",self.layer,self.sx, self.sy, self.tx, self.ty} )
    --print("sent scaletrans...\n")
	end
end


function Media:Fade (v,time)
	self.fade=v/127
	if not self.loaded then return end
	Out({"net",self.name,"send","fade",self.layer,self.fade,time})
end

--
-- retourne les messages midi pour output
--
function Media:midiState ()
	if self.loaded then vload=127 else vload=0 end

	vtx=midiround(self.tx*64+64)
	vty=midiround(self.ty*64+64)
	vsx=midiround(self.sx*64+64)
	vsy=midiround(self.sy*64+64)

	print("media dump midi ",vtx,vty,vsx,vsy,self.fade,vload)

	Out(	{"midiout",self.assign.Fade,midiround(self.fade*127)},
		{"midiout",self.assign.Load,vload},
		{"midiout",self.assign.Tx,vtx},
                {"midiout",self.assign.Ty,vty},
                {"midiout",self.assign.Sx,vsx},
                {"midiout",self.assign.Sy,vsy}
	 )
end


