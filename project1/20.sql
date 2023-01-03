SELECT count(*) number
FROM CaughtPokemon
JOIN Pokemon
ON CaughtPokemon.pid = Pokemon.id
WHERE Pokemon.type IN ('Water','Electric','Psychic')
ORDER BY number;
