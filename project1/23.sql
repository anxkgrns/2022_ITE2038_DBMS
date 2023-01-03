SELECT Pokemon.name 
FROM Pokemon
WHERE SUBSTR(Pokemon.name,1,1) IN ('A','E','I','O','U')
ORDER BY Pokemon.name ASC;
