
--
-- definit une classe...
--
--

--
-- une petite fonction pas rapport
--
function midiround(v)
	w=math.floor(v+0.5);
	if w<0 then return 0 end
	if w>127 then return 127 end
	return w
end



--
-- assigne la classe "class" a l'object obj
-- (pas besoin d'avoir un champ 'class' dans l'objet. Il sera defini si pas la.
-- le second argument (class) est optionel. Si il est absent, on
-- cherche un champ 'classe' dans l'objet
-- obj peut etre une classe... pas de probleme
--
function setClass(obj,class)
	if class then
		obj.class=class
		setmetatable(obj,{__index=obj.class})
	else
		if obj.class then
			setmetatable(obj,{__index=obj.class})
		else
			print("object has no class defined")
		end
	end
end

--
-- retourne la classe parente d'un objet ou classe
-- obj.class contient la classe de obj
-- si obj est une classe, alors obj.classe est la classe parente
-- si obj.class n'existe pas, c'est qu'on est au plus haut niveau
--
--function parent (obj)
--	c=obj.class
--	if c then
--		return c.class
--	end
--	return nil
--end


