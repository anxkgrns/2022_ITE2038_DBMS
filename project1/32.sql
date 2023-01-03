SELECT Trainer.name, Trainer.hometown
FROM CaughtPokemon
JOIN Trainer
ON CaughtPokemon.owner_id = Trainer.id
JOIN Pokemon
ON Pokemon.id = CaughtPokemon.pid
JOIN Gym
ON Gym.leader_id = Trainer.id
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
OR CaughtPokemon.pid NOT IN
(
  (SELECT after_id FROM Evolution) union (SELECT before_id FROM Evolution)
)
GROUP BY Trainer.name, Trainer.hometown
ORDER BY Trainer.name ASC;
