SELECT Trainer.name
FROM Trainer
WHERE Trainer.id IN 
(
  SELECT Gym.leader_id
  FROM GYM
  GROUP BY Gym.leader_id
)
ORDER BY Trainer.name ASC;
