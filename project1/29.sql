SELECT Trainer.name
FROM CaughtPokemon
JOIN Trainer
ON CaughtPokemon.owner_id = Trainer.id
JOIN Pokemon
ON Pokemon.id = CaughtPokemon.pid
WHERE Pokemon.type IN ('Psychic')
ORDER BY Trainer.name ASC;
