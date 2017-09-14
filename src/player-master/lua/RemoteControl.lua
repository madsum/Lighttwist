-- FONCTIONS pour controler le setup de projection de la caserne

function docmd (cmd)
       print("execute ",cmd)
       a=assert(loadstring(cmd))()
       return {a}
end

function start_back ()
         print("Depart des players de retro-projection")
         os.execute("ssh glam2 './play_yukie_1' &")
         os.execute("ssh glam2 './play_yukie_2' &")
         os.execute("ssh glam2 './play_yukie_3' &")
         os.execute("ssh glam5 './play_yukie_1' &")
         os.execute("ssh glam5 './play_yukie_2' &")
end

function start_front ()
         print("Depart des players de projection avec DL1")
         os.execute("ssh glam3 './play_yukie_1' &")
         os.execute("ssh glam3 './play_yukie_2' &")
end

function start_all ()
         start_back()
         start_front()
end

function kill_back ()
         print("Interruption des players de retro-projection")
         os.execute("ssh glam2 'killall ltplayerlua'")
         os.execute("ssh glam5 'killall ltplayerlua'")
end

function kill_front ()
         print("Interruption des players de projection avec DL1")
         os.execute("ssh glam3 'killall ltplayerlua'")
end

function kill_all ()
         kill_back()
         kill_front()
end

function dl1_plancher ()
         print("DL1 en position plancher")
         os.execute("ssh glam3 'dlxdmxcmd -dl1 0 41 -dl1 1 21 -dl1 2 1 < dl1plancher' &")
end

function dl1_plancher_1 ()
         print("DL1 en position plancher")
         os.execute("ssh glam3 'dlxdmxcmd -dl1 0 41 -dl1 1 21 -dl1 2 1 < dl1plancher_1' &")
end

function dl1_ecran0 ()
         print("DL1 en position pour ecran 0 (devant)")
         os.execute("ssh glam3 'dlxdmxcmd -dl1 0 41 -dl1 1 21 -dl1 2 1 < dl1ecran0' &")
end

function dl1_ecran1 ()
         print("DL1 en position pour ecran 1")
         os.execute("ssh glam3 'dlxdmxcmd -dl1 0 41 -dl1 1 21 -dl1 2 1 < dl1ecran1' &")
end

function dl1_ecran2 ()
         print("DL1 en position pour ecran 2 (fond)")
         os.execute("ssh glam3 'dlxdmxcmd -dl1 0 41 -dl1 1 21 -dl1 2 1 < dl1ecran2' &")
end

function dl1_ecran3 ()
         print("DL1 en position pour ecran 3 (diagonale)")
         os.execute("ssh glam3 'dlxdmxcmd -dl1 0 41 -dl1 1 21 -dl1 2 1 < dl1ecran3' &")
end

function dl1_ecran4 ()
         print("DL1 en position pour ecran 4 (perpendiculaire)")
         os.execute("ssh glam3 'dlxdmxcmd -dl1 0 41 -dl1 1 21 -dl1 2 1 < dl1ecran4' &")
end

function dl1_ecran5 ()
         print("DL1 en position pour ecran 5 (centre)")
         os.execute("ssh glam3 'dlxdmxcmd -dl1 0 41 -dl1 1 21 -dl1 2 1 < dl1ecran5' &")
end

function dlx( script )
    print( "dlxdmxcmd -dl1 0 41 -dl1 1 21 -dl1 2 1 < " .. script )
    os.execute( "ssh glam3 'dlxdmxcmd -dl1 0 41 -dl1 1 21 -dl1 2 1 < " .. script .. "' &" )
end

function trackHumans( dev, mode )
    print("ssh glam3 'trackHumans " .. dev .. " " .. mode ..  "  ' &")
    os.execute( "ssh glam3 'trackHumans " .. dev .. " " .. mode ..  "& ' &" )
end