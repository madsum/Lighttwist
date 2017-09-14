
--
-- This file contains the commands understood by the player
-- if we get the command:   aaa bbb ccc 1.5 xxx
-- then lua execute:
-- aaa("bbb","ccc","1.5","xxx")
--
-- Assume that the module player is loaded (player is the interface to the osgplayer itself)
--
-- commandes:
--
-- model <layer_num> <model_name_osg>   -> load an osg 3D model
-- movie <layer_num> <model_name_osg>	-> load a movie
-- crayon <layer_num>			-> load the drawing
-- unload <layer_num>
-- frame <layer_num> <frame_num>
--
--

if player then
        for k,v in pairs(player) do
                print("table player has fonction ",k)
        end
else
        print("table player IS NOT DEFINED!!!")
end


function bye()
    player.quit()
end


--
-- un test pour les commande avec extra binaire
--
function pain (bpp,x,y,w,h,bin)
	--print("[commands.lua] PAIN! ",bpp,x,y,w,h,bin);
	-- on paint toujours sur le layer 0 (pour l'instant...)
	player.paint(0,bpp,x,y,w,h,bin)
end



function loadAlpha(layer, fn)
  player.set_alpha_mask(layer, fn)
end


function enableLUT(layer, e)
  player.enableLUT(e)
end

function loadLutBlend(l, b)
     --int load_lut_blend(const char* lutName, const char* blendName);
     player.load_lut_blend(l,b)
end

function dl1track(layer)
   player.load_layer_dl1track(layer)
end

--
-- variables
--

--
-- chaque video qui est loade est dans cette liste, indexee par layer
-- chaque layer contient les infos suivantes:
--
-- start : contient l'heure de depart
-- fps : frames per seconds
-- play : true ou false selon qu'on avance ou pas...
--
video={}
crayonLoaded = nil

--
-- fonction appelle a chaque frame du viewer pour rafraichir...
--

function tick ()
	--print("TICK!")
	--player.getTime())

	for k,v in pairs(video) do
		--print("updating video layer ",k)
		if v.play then
			v.lastframe = (player.getTime() - v.start)*v.fps
			player.frame(k,math.floor(v.lastframe))
            --print("updating video layer ",k, "  frame=", v.lastframe, "   fps=", v.fps)
			--print("frame",k,v.lastframe,math.floor(v.lastframe))
		end
	end

end

--
--
--
--
-- commandes recues de l'exterieur
--
--
--
--

function plugin (layer, name, class, param)
    print("PLOGUINNE", layer, name, class, param)
    player.load_layer_add_plugin(layer, name, class, param)
end

function model (layer,file)
	print("MODEL",layer,file)
	player.load_layer_multi3d(layer,file)
end

function movietiled (layer,file)
	print("MOVIE",layer,file)
	player.load_layer_tiled_movie(layer,file)

	video[layer]={
			start=nil;
			fps=player.getFPS(layer);
			play=false;
			lastframe=nil;
		}
end

function movie(layer, file)
	print("MOVIE",layer,file)
	player.load_layer_movie(layer, file)

	video[layer]={
			start=nil;
			fps=player.getFPS(layer);
			play=false;
			lastframe=nil;
		}
end

function moviewrap(layer, file, swrap, twrap)
	print("MOVIE",layer,file)
	player.load_layer_movie(layer, file, swrap, twrap)

	video[layer]={
			start=nil;
			fps=player.getFPS(layer);
			play=false;
			lastframe=nil;
		}
end

function unload (layer)
	print("UNLOAD",layer)
	player.unload_layer(layer)
  if (layer == crayonLoaded) then
    crayonLoaded = nil
	end
  video[layer]=nil;
end

--function frame (layer,num)
--	print("FRAME",layer,num)
--	player.frame(layer,num)
--end

function image (layer,file)
	print("IMAGE",layer,file)
	player.load_layer_image(layer,file)
end

function imagewrap (layer, file, swrap, twrap )
	print("IMAGE",layer,file)
	player.load_layer_image(layer, file, swrap, twrap )
end



function fade_out_all (f)
	player.fade_out_all(f)
end

function fade_in_all (f)
	player.fade_in_all(f)
end


function fade (layer,f)
	print("FADE:",layer,":",f,":",player.getTime())
	player.fade(layer,f)
end

-- To change the clear node colour
function background (r,g,b)
	--print("CLEAR NODE set to (",r,g,b,"1.0)")
	player.set_clear_node(r,g,b);
end

function color (layer, r, g, b, a, mode)
	print("DE LA COULEUR! OUUUHHH!",layer)
	player.load_layer_color(layer, r, g, b, a, mode)
end

function set_color(layer, r, g, b, a)
    player.set_color(layer, r, g, b, a)
end


function ultimate (layer, camid)
	print("ULTIMATE PLUGIN LAYER OF DOOOOOOM!!!!11!1!!!one11one", layer, camid)
	player.load_layer_ultime(layer,camid)
end

function streamin (layer,port,mtu)
	print("STREAMED CAPTURE",layer)
	player.load_layer_stream_capture(layer,port,mtu)
end

function capture (layer,camid,exposure,fps)
	print("PROSILICA CAPTURE",layer)
	player.load_layer_capture(layer,camid)
end

function captureblend (layer,camid,ratio)
    print("PROSILICA CAPTURE with PLUGIN frameblend",layer) 
    player.load_layer_capture_with_blend(layer,camid,ratio)
end

function captureinverse (layer,camid)
    print("PROSILICA CAPTURE with PLUGIN inverse",layer) 
    player.load_layer_capture_with_inverse(layer,camid)
end

function capturedistorsion (layer,camid,mapfilename)
    print("PROSILICA CAPTURE with PLUGIN de distorsion",layer) 
    player.load_layer_capture_with_distorsion(layer,camid,mapfilename)
end

function capturescanner (layer,camid)
    print("PROSILICA CAPTURE with PLUGIN scanner",layer) 
    player.load_layer_capture_with_scanner(layer,camid)
end

function capturecrayon (layer)
	 player.load_layer_camera_crayon(layer)
end

function capturefiltre (layer,camid,str_filter)
    print("PROSILICA CAPTURE with PLUGINS",layer," ") 
    
    player.load_layer_capture_with_plugins(layer,camid,"inverse filter:frameblend filter -ratio 0.8:distorsion gpufilter -map /home/vision/svn3d/danieldanis/demo/distorsion/mosaique.png -width 659 -height 493:crayon filter")
end

function crayon (layer)
	print("CRAYON",layer)
  crayonLoaded = layer
	player.load_layer_crayon(layer)
end


function scaletrans (layer,sx,sy,tx,ty)
        print("SCALETRANS",layer,sx,sy,tx,ty)
        player.scaletrans(layer,sx,sy,tx,ty)
end

function drawpt(x, y, z)
        if (crayonLoaded == nil) then
          --print("CRAYON NOT LOADED... IGNORING")
          return
        end
        
        --x = x * 10 + 10;
        --y = y * 2;
        --z = z * 10 + 10;
        print("QUEUEPOINT",x, y, z)
        player.queuepoint(x, y, z)
end

function newLine()
         if (crayonLoaded == nil) then 
         return
        end
        print("NEWLINE")
         player.newline()     
end

function newPage()
         if (crayonLoaded == nil) then 
         return
        end
        print("NEWPAGE")
         player.newpage()     
end

-- ptTrigger a ete cree pour le son
--function ptTrigger(n)
--    if n == 0 then
--          newLine()
--    end
--end

--
-- frame = (gettime() - start) * fps
--
-- f = (t-s)*fps = t*fps - s*fps 
-- f + s*fps = t*fps
-- s = (t*fps - f) / fps = t - f/fps
--


function play (layer,cmd)
	print("PLAY",layer,device)

	v=video[layer]

	if v==nil then return end

	if cmd=="start" then
		if v.player then return end
		if v.lastframe then
			-- reset le start pour etre au bon frame
			v.start= player.getTime() - v.lastframe/v.fps
		else
			-- new start!
			v.start= player.getTime()
		end
		v.play=true
		print("starting play layer ",layer, " temps = ", v.start)
	elseif cmd=="stop" then
		v.play=false
		print("stoping play layer ",layer)
	end
end

function fps (layer,nf)
	print("FPS",layer,nf)

	ff=nf+0

	v=video[layer]
	if v==nil then return end

	-- fps' est le nouveau fps et s' le nouveau start qui donne le meme f
	-- frame = (time-start')*fps' = time*fps' - start'*fps'
	-- start'*fps' + frame = time*fps'
	-- start' = (time*fps' - frame)/fps' = fime - frame/fps'
	--
	if ff<=0 then ff=0.01 end
	if v.play and v.lastframe then
		v.start=player.getTime() - v.lastframe / ff
	end
	v.fps=ff
end


function time(t)
	print("TIME reference ",t)
	player.setTime(t)

end


--
-- Le super effet de scanner si on est en train de capturer...
--

function set_plugin_param(layer, plugname, paramname, ...)
    print("SETTING PARAM:", layer, plugname, paramname, " [", table.concat(arg, " "), "]")
    k = player.set_plugin_param(layer, plugname, paramname, table.concat(arg, " "))

end


function invoke_plugin_command(layer, plugname, ...)
    print("INVOKING COMMAND:", layer, plugname, " [", table.concat(arg, " "), "]")
    k = player.invoke_plugin_command(layer, plugname, table.concat(arg, " "))

end


function canny(layer,onoff)
    if onoff=="1" then
        player.invoke_plugin_command(layer,"canny","ON")
		print("CANNY ON")
	else
	    player.invoke_plugin_command(layer,"canny","OFF")
		print("CANNY OFF")
	end
end

function inverse(layer,onoff)
    if onoff=="1" then
        player.invoke_plugin_command(layer,"inverse","ON")
		print("INVERSE ON")
	else
	    player.invoke_plugin_command(layer,"inverse","OFF")
		print("INVERSE OFF")
	end
end

function scanner(layer,onoff)
    if onoff=="1" then
        player.invoke_plugin_command(layer,"scannereffect","ON")
		print("SCANNER ON")
	else
	    player.invoke_plugin_command(layer,"scannereffect","OFF")
		print("SCANNER OFF")
	end
end


function setmap(layer,mapfilename)
		 player.set_plugin_param(layer, "distorsion", "Map", mapfilename)	
		 print("Changing map to ",mapfilename)
end

function go()
         --rien
end

function unload4to16()

    for i = 4,15 do
	    player.unload_layer(i)
    end
end

function unloadAll()

    --for i = 0,31 do
    --    player.unload_layer(i)
    --end
    player.unload_all()
end


function trackHumans( dev )
    print("trackHumans!!")
    os.execute( "trackHumans " .. dev .. " &" )
end

function dlx( script )
    print( "dlxdmxcmd -dl1 0 41 -dl1 1 21 -dl1 2 1 < " .. script )
    os.execute( "dlxdmxcmd -dl1 0 41 -dl1 1 21 -dl1 2 1 < " .. script )
end