SELECT A.trainer, total
FROM (
  SELECT Trainer.name trainer, SUM(CaughtPokemon.level) total
  FROM CaughtPokemon
  JOIN Trainer
  ON Trainer.id = CaughtPokemon.owner_id
  GROUP BY trainer
  ORDER BY trainer
) as A
WHERE A.total = (SELECT Max(A.total) FROM (
  SELECT Trainer.name t, SUM(CaughtPokemon.level) total
  FROM CaughtPokemon
  JOIN Trainer
  ON Trainer.id = CaughtPokemon.owner_id
  GROUP BY t
  ORDER BY t
) as A) 
ORDER BY A.trainer ASC;
