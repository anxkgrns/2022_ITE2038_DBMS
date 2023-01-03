SELECT Pokemon.name
FROM Pokemon, CaughtPokemon
WHERE Pokemon.id NOT IN
(
  SELECT CaughtPokemon.pid 
  FROM CaughtPokemon
  GROUP BY CaughtPokemon.pid
)
GROUP BY Pokemon.name
ORDER BY Pokemon.name ASC;
