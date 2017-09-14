require("Media")

--
-- Classe Image... contient un fichier, et c'est tout.
-- normalement on ajouterai le tx,ty,sx,sy par exemple
--
-- file
-- tx,ty,sx,sy
--

--
-- les classes sont definies ici au depart... vide... ou presque
--
Image=	{}
setClass(Image,Media)


function Image:init ()
	Media.init(self)
end

function Image:dump ()
	Media.dump(self)
	print("Image: file",self.file)
end

--
--
-- les controles midi
-- appelle avec (self,midi_val,time)
--
--

-- play est non utilise... juste pour le mettre a off
function Image:Play (v,time)
	Out( {"midiout",self.assign.Play,0} )
end


function  Image:midiState ()
	Media.midiState(self)
	Out( {"midiout",self.assign.Play,0} )
end


--
-- special... normalement dans le Media mais bon...
--
function Image:Load (v,time)
	Media.Load(self,v,time)
	if self.loaded then
		Out( {"net",self.name,"sendsafe","image",self.layer,self.file} )
		self:Fade(0,time)
	else
		Out( {"net",self.name,"sendsafe","unload",self.layer} )
    Out( {"net",self.name,"send","scaletrans",self.layer,self.sx, self.sy, self.tx, self.ty} )
	end
	self:midiState()
end


