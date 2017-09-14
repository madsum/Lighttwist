require("Media")

--
-- Classe Model... contient un modele 3D osg, et c'est tout.
-- normalement on ajouterai des parametres a animer par exemple
--
-- file
-- factor : un exemple de parametre
--

--
-- les classes sont definies ici au depart... vide... ou presque
--
Model=	{}
setClass(Model,Media)


function Model:init ()
	Media.init(self)
  --	if not self.factor then self.factor=1 end
end

function Model:dump ()
	Media.dump(self)
	print("Model: file",self.file)
	--print("Model: factor",self.factor)
end

--
--
-- les controles midi
-- appelle avec (self,midi_val,time)
--
--

function Model:Factor (v,time)
  --      self.factor=v
	--if self.loaded then
	--	Out( {"net",self.name,"send","factor",self.layer,self.factor} )
	--end
end

-- play est non utilise... juste pour le mettre a off
function Model:Play (v,time)
	Out( {"midiout",self.assign.Play,0} )
end


function  Model:midiState ()
	Media.midiState(self)
--	{"midiout",self.assign.Factor,self.factor},
	Out(
		{"midiout",self.assign.Play,0} )
end


--
-- special... normalement dans le Media mais bon...
--
function Model:Load (v,time)
	Media.Load(self,v,time)
	if self.loaded then
		Out( {"net",self.name,"sendsafe","model",self.layer,self.file} )
    Out( {"net",self.name,"send","scaletrans",self.layer,self.sx, self.sy, self.tx, self.ty} )
		self:Fade(0,time)
	else
		Out( {"net",self.name,"sendsafe","unload",self.layer} )
	end
	self:midiState()
end


