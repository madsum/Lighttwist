require("Media")

--
-- Classe Movie... contient un movie, avec un fps...
--
-- file
-- fps
-- playing  (intern state)
--

--
-- les classes sont definies ici au depart... vide... ou presque
--
Movie=	{}
setClass(Movie,Media)


function Movie:init ()
	Media.init(self)
	self.playing=false
	if not self.fps then self.fps=30 end
end

function Movie:dump ()
	Media.dump(self)
	print("Movie: file",self.file)
	print("Movie: fps",self.fps)
	print("Movie: playing",self.playing)
end

--
--
-- les controles midi
-- appelle avec (self,midi_val,time)
--
--

function Movie:Fps (v,time)
        self.fps=v
	if self.loaded then
		Out( {"net",self.name,"send","fps",self.layer,self.fps,time} )
	end
end

function Movie:Play (v,time)
        if v<64 then
                self.playing=false
		Out( {"net",self.name,"sendsafe","play",self.layer,"stop"} )
        else
		-- on doit faire load si c'est pas deja fait...
		if not self.loaded then
			self:Load(127,time)
		end
                self.playing=true
		self:midiState()
		Out(	{"net",self.name,"send","fps",self.layer,self.fps,time},
			{"net",self.name,"sendsafe","play",self.layer,"start",time} )
	end
end

function  Movie:midiState ()
	if self.playing then vplay=127 else vplay=0 end

	print("output midi of media");
	Media.midiState(self)
	print("output midiout ",vplay,self.fps);
	Out(	{"midiout",self.assign.Play,vplay},
		{"midiout",self.assign.Fps,self.fps} )
end


--
-- special... normalement dans le Media mais bon...
--
function Movie:Load (v,time)
	Media.Load(self,v,time)
	if self.loaded then
		Out( {"net",self.name,"sendsafe","movie",self.layer,self.file} )
    Out( {"net",self.name,"send","scaletrans",self.layer,self.sx, self.sy, self.tx, self.ty} )
		self:Fade(0,time)
	else
		-- arrete de jouer si necessaire
		self:Play(0,time)
		Out( {"net",self.name,"sendsafe","unload",self.layer} )
	end
	self:midiState()
end


