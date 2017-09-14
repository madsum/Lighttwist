require("Media")
--require("os")

--
-- Classe Capture... contient un fichier, et c'est tout.
-- normalement on ajouterai le tx,ty,sx,sy par exemple
--
-- file
-- tx,ty,sx,sy
--

--
-- les classes sont definies ici au depart... vide... ou presque
--
Capture=	{}
setClass(Capture,Media)


function Capture:init ()
	Media.init(self)
	-- assigne dans les controles, si pas deja defini
	if not self.camid then self.camid=-1 end
	if not self.exposure then self.exposure=-1 end
	if not self.fps then self.fps=-1 end
	if not self.tx then self.tx=0 end
	if not self.ty then self.ty=0 end
	if not self.sx then self.sx=1 end
	if not self.sy then self.sy=1 end
	if not self.fade then self.fade=0 end
	if not self.scan then self.scan=0 end
	if not self.inverse then self.inverse=0 end
	if not self.canny then self.canny=0 end
	if not self.freeze then self.freeze=0 end
end

function Capture:dump ()
	Media.dump(self)
	print("Capture: camid",self.camid)
	print("Capture: exposure",self.exposure)
	print("Capture: fade",self.fade)
	print("Capture: fps",self.fps)
	print("Capture: tx",self.tx)
	print("Capture: ty",self.ty)
	print("Capture: sx",self.sx)
	print("Capture: sy",self.sy)
end

--
--
-- les controles midi
-- appelle avec (self,midi_val,time)
--
--

function Capture:Canny (v,time)
	if v>0 then self.canny=1 else self.canny=0 end
	if self.loaded then
		Out( {"net",self.name,"send","canny",self.layer,self.canny} )
	end
end

function Capture:Scanner (v,time)
	if v>0 then self.scan=1 else self.scan=0 end
	if self.loaded then
		Out( {"net",self.name,"send","scanner",self.layer,self.scan} )
	end
end


function Capture:Inverse (v,time)
	if v>0 then self.inverse=1 else self.inverse=0 end
	if self.loaded then
		Out( {"net",self.name,"send","inverse",self.layer,self.inverse} )
	end
end

function Capture:Freeze (v,time)
	if v>0 then self.freeze=1 else self.freeze=0 end
	if self.loaded then
		Out( {"net",self.name,"send","freeze",self.layer,self.freeze} )
	end
end

-- play est non utilise... juste pour le mettre a off
function Capture:Play (v,time)
	Out( {"midiout",self.assign.Play,0} )
end


function  Capture:midiState ()
	vtx=midiround(self.tx*16+64)
	vty=midiround(self.ty*16+64)
	vsx=midiround(128.0/self.sx-1)
	vsy=midiround(128.0/self.sy-1)

	Media.midiState(self)
	Out(	{"midiout",self.assign.Tx,vtx},
		{"midiout",self.assign.Ty,vty},
		{"midiout",self.assign.Sx,vsx},
		{"midiout",self.assign.Sy,vsy},
		{"midiout",self.assign.Play,0} )
end


--
-- special... normalement dans le Media mais bon...
--
function Capture:Load (v,time)


	Media.Load(self,v,time)
	if self.loaded then
	   --Out( {"net",self.name,"sendsafe","capture",self.layer,self.camid,self.exposure,self.fps} )
		Out( {"net",self.name,"sendsafe","capturefiltre",self.layer,self.camid,"inverse:distorsion -map /home/vision/svn3d/danieldanis/demo/distorsion/mosaique.png -width 659 -height 493:crayon:frameblend"} )
         --Out( {"net",self.name,"sendsafe","capturefiltre",self.layer,self.camid,"canny:inverse:scannereffect"} )
		Out( {"net",self.name,"send","scaletrans",self.layer,self.sx, self.sy, self.tx, self.ty} )
		--os.execute("ssh lavette 'killall imguview;export LD_LIBRARY_PATH=/usr/local/lib;imguview -geom 0 0 800 480 false -camera gige:mcast=1:camid=" .. self.camid .. " > /dev/null' &")
		--print("ssh lavette 'killall imguview;export LD_LIBRARY_PATH=/usr/local/lib;imguview -geom 0 0 800 480 false -camera gige:camid=" .. self.camid .. " > /dev/null' &")
	    self:Fade(0,time)
	    self:Canny(self.canny,time)
	    self:Inverse(self.inverse,time)
	    self:Scanner(self.scan,time)
	else
		Out( {"net",self.name,"sendsafe","unload",self.layer} )
		os.execute("ssh lavette 'export LD_LIBRARY_PATH=/usr/local/lib;killall imguview'")
	end
	print("------------------ Capture:Load")
	self:midiState()

	self:dump()

	--print("testing os.time ", os.date())
	--print("testing os.getenv ", os.getenv("LD_LIBRARY_PATH"))
	
end


