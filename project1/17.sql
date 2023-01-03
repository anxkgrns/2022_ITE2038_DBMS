SELECT AVG(level) average
FROM CaughtPokemon
JOIN Pokemon
ON CaughtPokemon.pid = Pokemon.id
WHERE Pokemon.type IN ('Water' )
ORDER BY average;
