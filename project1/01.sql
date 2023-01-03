SELECT Trainer.name, count(*) count 
FROM CaughtPokemon 
JOIN Trainer
ON CaughtPokemon.owner_id = Trainer.id
GROUP BY Trainer.name
HAVING count(*)>=3
ORDER BY count(*) ASC;

