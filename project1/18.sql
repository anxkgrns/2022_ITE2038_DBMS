SELECT Trainer.name,SUM(level) level_total
FROM CaughtPokemon
JOIN Trainer
ON CaughtPokemon.owner_id = Trainer.id
WHERE Trainer.id IN 
(
  SELECT Gym.leader_id
  FROM GYM
  GROUP BY Gym.leader_id
)
GROUP BY Trainer.name
ORDER BY level_total DESC, Trainer.name ASC;
