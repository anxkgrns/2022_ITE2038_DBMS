SELECT SUM(CaughtPokemon.level)
FROM CaughtPokemon
JOIN Trainer
ON CaughtPokemon.owner_id = Trainer.id
JOIN Pokemon
ON Pokemon.id = CaughtPokemon.pid
WHERE CaughtPokemon.pid 
NOT IN
(
  (SELECT after_id FROM Evolution) union (SELECT before_id FROM Evolution)
)
ORDER BY sum ASC;
