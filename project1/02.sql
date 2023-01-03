SELECT Trainer.name 
FROM Trainer
WHERE Trainer.id NOT IN 
(
  SELECT leader_id 
  FROM GYM
)
ORDER BY Trainer.name ASC;
