SELECT Pokemon.name 
FROM Pokemon
WHERE Pokemon.id IN 
(
  SELECT after_id 
  FROM Evolution 
  WHERE before_id NOT IN 
  (
    SELECT after_id 
    FROM Evolution
  )
) 
ORDER BY Pokemon.name ASC;
