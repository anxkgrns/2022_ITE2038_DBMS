SELECT Trainer.name FROM CaughtPokemon
JOIN Trainer
ON Trainer.id = CaughtPokemon.owner_id
WHERE CaughtPokemon.owner_id IN (
  SELECT owner_id
  FROM CaughtPokemon
  GROUP BY owner_id,pid
  HAVING count(*)>=2
)
GROUP BY Trainer.name
ORDER BY Trainer.name ASC;
