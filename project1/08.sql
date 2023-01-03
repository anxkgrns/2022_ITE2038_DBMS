SELECT Trainer.name, AVG(CaughtPokemon.level) average 
FROM CaughtPokemon
JOIN Trainer
ON CaughtPokemon.owner_id = Trainer.id
JOIN Pokemon
ON Pokemon.id = CaughtPokemon.pid
WHERE Pokemon.type IN ('Normal', 'Electric')
GROUP BY Trainer.name
ORDER BY average ASC,Trainer.name ASC;  
