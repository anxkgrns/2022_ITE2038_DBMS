SELECT Trainer.hometown,CaughtPokemon.nickname 
FROM (
  SELECT Trainer.hometown,Max(level) maxlevel
  FROM CaughtPokemon
  JOIN Trainer
  ON Trainer.id = CaughtPokemon.owner_id
  GROUP BY Trainer.hometown
) as A
JOIN Trainer
ON Trainer.hometown = A.hometown
JOIN CaughtPokemon
ON CaughtPokemon.owner_id =Trainer.id AND CaughtPokemon.level = A.maxlevel
ORDER BY Trainer.hometown ASC;
