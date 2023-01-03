SELECT Trainer.name, City.description
FROM CaughtPokemon
JOIN Trainer
ON CaughtPokemon.owner_id = Trainer.id
JOIN Pokemon
ON Pokemon.id = CaughtPokemon.pid
JOIN City
ON City.name = Trainer.hometown
WHERE Pokemon.type = ('Fire')
GROUP BY Trainer.name, City.description
ORDER BY Trainer.name ASC;
