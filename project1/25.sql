SELECT Trainer.name, Pokemon.type, count(*) count
FROM CaughtPokemon
JOIN Pokemon
ON Pokemon.id = CaughtPokemon.pid
JOIN Trainer
ON CaughtPokemon.owner_id = Trainer.id
GROUP BY Trainer.name, Pokemon.type
ORDER BY Trainer.name ASC, Pokemon.type ASC;
