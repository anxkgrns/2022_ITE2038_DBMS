SELECT Trainer.name, CaughtPokemon.nickname
FROM CaughtPokemon
JOIN Trainer
ON CaughtPokemon.owner_id = Trainer.id
JOIN 
(
  SELECT CaughtPokemon.owner_id ownerid, MAX(CaughtPokemon.level)
  FROM CaughtPokemon
  GROUP BY ownerid
) as A
ON A.ownerid = Trainer.id
WHERE Trainer.name IN
(
  SELECT Trainer.name
  FROM Trainer
  JOIN CaughtPokemon
  ON CaughtPokemon.owner_id = Trainer.id
  GROUP BY Trainer.name
  HAVING count(*)>=3
)
AND A.max = CaughtPokemon.level
GROUP BY Trainer.name, CaughtPokemon.nickname
ORDER BY Trainer.name ASC, CaughtPokemon.nickname ASC;
