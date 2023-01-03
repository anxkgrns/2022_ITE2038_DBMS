SELECT Pokemon.name 
FROM Pokemon
WHERE Pokemon.id IN 
(
  SELECT before_id 
  FROM Evolution 
  WHERE before_id > after_id
)
GROUP BY Pokemon.name
ORDER BY Pokemon.name ASC;
