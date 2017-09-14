require("Class")

--
-- Classe Player... contient un groupe de layers,
-- avec lequel on communique sur un port donne.
--
-- name : nom du player (ex: plancher, plafond, background)
-- loaded	(intern state)
--

--
-- les classes sont definies ici au depart... vide...
--
Player={}

function Player:init ()
	self.loaded=false
	if not self.name then self.name="default" end
end

function Player:dump ()
	print("Player: name",self.name)
	print("Player: loaded",self.loaded)
end

--
--
-- fonctions parametres (appele par controles midi) --
-- elles commencent par une majuscule...
-- elle sont appellees avec les parametres midi_value,time
--



