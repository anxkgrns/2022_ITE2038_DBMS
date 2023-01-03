SELECT Trainer.name,Trainer.hometown
FROM CaughtPokemon
JOIN Trainer
ON CaughtPokemon.owner_id = Trainer.id
JOIN Evolution
ON Evolution.before_id = CaughtPokemon.pid
JOIN
(
  SELECT Trainer.name as name,CaughtPokemon.pid as pid
  FROM CaughtPokemon
  JOIN Trainer
  ON CaughtPokemon.owner_id = Trainer.id
) as A
ON Trainer.name = A.name
AND A.pid = Evolution.after_id
GROUP BY Trainer.name,Trainer.hometown
ORDER BY Trainer.name ASC;
