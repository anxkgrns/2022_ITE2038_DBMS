SELECT COUNT(*)
FROM Pokemon
WHERE Pokemon.type NOT IN ('Grass' )
ORDER BY COUNT(*);
