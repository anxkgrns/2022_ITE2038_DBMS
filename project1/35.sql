SELECT Trainer.name, SUM(CaughtPokemon.level)
FROM CaughtPokemon
JOIN Trainer
ON CaughtPokemon.owner_id = Trainer.id
JOIN City
ON City.name = Trainer.hometown
WHERE Trainer.hometown = ('Blue City')
AND CaughtPokemon.pid IN
(
  (SELECT after_id FROM Evolution) union (SELECT before_id FROM Evolution)
)
GROUP BY Trainer.name
ORDER BY Trainer.name ASC;
