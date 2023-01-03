SELECT Pokemon.id, CaughtPokemon.nickname
FROM CaughtPokemon
JOIN Trainer
ON CaughtPokemon.owner_id = Trainer.id
JOIN Pokemon
ON Pokemon.id = CaughtPokemon.pid
JOIN City
ON City.name = Trainer.hometown
WHERE Pokemon.type IN ('Water')
AND Trainer.homeTown IN ('Blue City')
ORDER BY Pokemon.id ASC,CaughtPokemon.nickname ASC;
