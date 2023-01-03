SELECT Pokemon.name, CaughtPokemon.level
FROM CaughtPokemon
JOIN Trainer
ON CaughtPokemon.owner_id = Trainer.id
JOIN Gym
ON Gym.leader_id = Trainer.id
JOIN Pokemon
ON Pokemon.id = CaughtPokemon.pid
WHERE Gym.city IN ('Sangnok City')
ORDER BY CaughtPokemon.level ASC, Pokemon.name ASC;
