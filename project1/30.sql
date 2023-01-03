SELECT MAX(CaughtPokemon.level) - MIN(CaughtPokemon.level) as difference
FROM CaughtPokemon
JOIN Pokemon
ON Pokemon.id = CaughtPokemon.pid
ORDER BY difference ASC;
