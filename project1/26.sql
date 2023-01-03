SELECT Trainer.name, Pokemon.name, count(Pokemon.name) count
FROM CaughtPokemon
JOIN Pokemon
ON Pokemon.id = CaughtPokemon.pid
JOIN Trainer
ON CaughtPokemon.owner_id = Trainer.id
WHERE Trainer.name IN 
(
  SELECT Trainer.name
  FROM CaughtPokemon
  JOIN Pokemon
  ON Pokemon.id = CaughtPokemon.pid
  JOIN Trainer
  ON CaughtPokemon.owner_id = Trainer.id
  GROUP BY Trainer.name
  HAVING count(DISTINCT Pokemon.type) =1 
)
GROUP BY Trainer.name, Pokemon.name
ORDER BY Trainer.name ASC, Pokemon.name ASC;
