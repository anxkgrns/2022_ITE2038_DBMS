SELECT Pokemon.name 
FROM Pokemon
JOIN CaughtPokemon
ON CaughtPokemon.pid = Pokemon.id
WHERE CaughtPokemon.owner_id IN
(
  SELECT Trainer.id
  FROM Trainer
  WHERE Trainer.hometown IN ('Sangnok City','Brown City')
  GROUP BY Trainer.id
)
GROUP BY Pokemon.name
ORDER BY Pokemon.name ASC;
