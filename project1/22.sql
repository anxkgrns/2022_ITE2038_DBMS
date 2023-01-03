SELECT Trainer.name
FROM CaughtPokemon
JOIN Trainer
ON CaughtPokemon.owner_id = Trainer.id
WHERE Trainer.id IN
(
  SELECT CaughtPokemon.owner_id
  FROM CaughtPokemon
  JOIN Pokemon
  ON CaughtPokemon.pid = Pokemon.id
  WHERE Pokemon.type IN ('Water')
  GROUP BY CaughtPokemon.owner_id
  HAVING count(CaughtPokemon.pid)>=2
)
GROUP BY Trainer.name
ORDER BY Trainer.name ASC;
