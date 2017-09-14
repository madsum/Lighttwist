require("Media")

--
-- Classe Crayon... contient un modele 3D osg, et c'est tout.
-- normalement on ajouterai des parametres a animer par exemple
--
-- factor : un exemple de parametre
--

--
-- les classes sont definies ici au depart... vide... ou presque
--
Crayon=	{}
setClass(Crayon,Media)


function Crayon:init ()
	Media.init(self)
end

function Crayon:dump ()
	Media.dump(self)
end

--
--
-- les controles midi
-- appelle avec (self,midi_val,time)
--
--

-- play est non utilise... juste pour le mettre a off
function Crayon:Play (v,time)
	Out( {"midiout",self.assign.Play,0} )
end


function  Crayon:midiState ()
	Media.midiState(self)
	Out( {"midiout",self.assign.Play,0} )
end


--
-- special... normalement dans le Media mais bon...
--
function Crayon:Load (v,time)
	Media.Load(self,v,time)
	if self.loaded then
		Out( {"net",self.name,"sendsafe","crayon",self.layer} )
    Out( {"net",self.name,"send","scaletrans",self.layer,self.sx, self.sy, self.tx, self.ty} )
		self:Fade(0,time)
	else
		Out( {"net",self.name,"sendsafe","unload",self.layer} )
	end
	self:midiState()

	print("----------------Crayon LOADED-----------------------")

end


