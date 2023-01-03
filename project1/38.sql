SELECT Trainer.name, count(*)
FROM CaughtPokemon
JOIN Trainer
ON CaughtPokemon.owner_id = Trainer.id
JOIN Pokemon
ON Pokemon.id = CaughtPokemon.pid
GROUP BY Trainer.name
HAVING count(DISTINCT Pokemon.type) = 2 
ORDER BY Trainer.name ASC;
