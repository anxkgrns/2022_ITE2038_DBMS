SELECT SUM(CaughtPokemon.level)
FROM CaughtPokemon
JOIN Trainer
ON CaughtPokemon.owner_id = Trainer.id
JOIN Pokemon
ON Pokemon.id = CaughtPokemon.pid
WHERE Pokemon.type NOT IN ('Fire','Grass','Water','Electric')
ORDER BY sum;
