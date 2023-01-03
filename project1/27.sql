SELECT Trainer.name, coalesce(SUM(A.level),0) sum
FROM CaughtPokemon
JOIN Trainer
ON CaughtPokemon.owner_id = Trainer.id
JOIN Gym
ON Gym.leader_id = Trainer.id
LEFT JOIN 
(
  SELECT CaughtPokemon.owner_id, CaughtPokemon.level
  FROM CaughtPokemon
  WHERE CaughtPokemon.level >= 50 
  GROUP BY CaughtPokemon.owner_id, CaughtPokemon.level
) as A
ON CaughtPokemon.level = A.level
AND CaughtPokemon.owner_id = A.owner_id
GROUP BY Trainer.name
ORDER BY Trainer.name ASC;
