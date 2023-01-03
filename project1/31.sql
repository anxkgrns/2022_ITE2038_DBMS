SELECT Trainer.name
FROM CaughtPokemon
JOIN Trainer
ON CaughtPokemon.owner_id = Trainer.id
WHERE CaughtPokemon.level
IN 
(
  SELECT MAX(CaughtPokemon.level)
  FROM CaughtPokemon
)
OR CaughtPokemon.level IN 
(
  SELECT MIN(CaughtPokemon.level)
  FROM CaughtPokemon
)
ORDER BY Trainer.name ASC;
