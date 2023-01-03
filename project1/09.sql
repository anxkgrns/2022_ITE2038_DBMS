SELECT Trainer.name FROM CaughtPokemon
JOIN Trainer
ON CaughtPokemon.owner_id = Trainer.id
JOIN Pokemon
ON Pokemon.id = CaughtPokemon.pid
WHERE CaughtPokemon.pid IN 
(
  SELECT after_id 
  FROM Evolution
  WHERE after_id 
  NOT IN 
  (
    SELECT before_id FROM Evolution
  )
)
GROUP BY Trainer.name
ORDER BY Trainer.name ASC;
