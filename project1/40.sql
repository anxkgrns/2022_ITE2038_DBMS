SELECT Trainer.name, City.description
FROM CaughtPokemon
JOIN Trainer
ON CaughtPokemon.owner_id = Trainer.id
JOIN Pokemon
ON Pokemon.id = CaughtPokemon.pid
JOIN City
ON City.name = Trainer.hometown
JOIN Gym
ON Gym.leader_id = Trainer.id
WHERE CaughtPokemon.pid IN
(
  SELECT after_id 
  FROM Evolution
)
AND Pokemon.type IN ('Fire','Grass','Water')
GROUP BY Trainer.name, City.description
ORDER BY Trainer.name ASC;
