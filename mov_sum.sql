CREATE TABLE Result(
    Original DOUBLE(30,10) NOT NULL,
    Answer DOUBLE(30,10) NOT NULL
);

DELIMITER $$

CREATE PROCEDURE moving_sum(table_name VARCHAR(15),column_name VARCHAR(15),H INT,J INT)
BEGIN

	DECLARE N INT DEFAULT 0;  -- N_row
	DECLARE i INT DEFAULT 0;
	DECLARE k INT DEFAULT 0;
	DECLARE prev_index INT DEFAULT 0;
	DECLARE next_index INT DEFAULT 0;
	DECLARE it_data DOUBLE(30,10);
	DECLARE prev_data DOUBLE(30,10);
	DECLARE next_data DOUBLE(30,10);
	DECLARE recent_data DOUBLE(30,10);
	DECLARE start_data DOUBLE(30,10);
	DECLARE end_data DOUBLE(30,10);
	DECLARE sum DOUBLE(30,10) DEFAULT 0;

	SELECT COUNT(*) FROM data INTO N;

	SET i = 0;
	SET k = 0;

	SELECT `x` FROM data LIMIT 0,1 INTO start_data;
	SELECT `x` FROM data LIMIT 499,1 INTO end_data;
	SET sum = H*start_data;
	SET prev_data = start_data;

	IF J-N+1<=0 THEN

		WHILE k<=J DO

			SELECT `x` FROM data LIMIT k,1 INTO it_data;
			SET sum = sum+it_data;
			SET k = k+1;

		END WHILE;

	ELSE 

		WHILE k<N DO

			SELECT `x` FROM data LIMIT k,1 INTO it_data;
			SET sum = sum+it_data;
			SET k = k+1;

		END WHILE;

		SET sum = sum+(J-N+1)*end_data;

	END IF;
	INSERT INTO Result VALUES (start_data,sum); 
	SET i = i+1;

	WHILE i<=H DO

		SET next_index = i+J;
		SELECT `x` FROM data LIMIT next_index,1 INTO next_data;
		SET sum = sum-prev_data+next_data;

		SELECT `x` FROM data LIMIT i,1 INTO recent_data;
		INSERT INTO Result VALUES (recent_data,sum);
		SET i = i+1;

	END WHILE;

	WHILE i+J<N DO

		SET prev_index = i-H-1;
		SET next_index = i+J;
		SELECT `x` FROM data LIMIT prev_index,1 INTO prev_data;
		SELECT `x` FROM data LIMIT next_index,1 INTO next_data;
		SET sum = sum-prev_data+next_data;

		SELECT `x` FROM data LIMIT i,1 INTO recent_data;
		INSERT INTO Result VALUES (recent_data,sum);
		SET i = i+1;

	END WHILE;

	WHILE i<N DO

		SET prev_index = i-H-1;
		SET next_data = end_data;
		SELECT `x` FROM data LIMIT prev_index,1 INTO prev_data;
		SET sum = sum-prev_data+next_data;

		SELECT `x` FROM data LIMIT i,1 INTO recent_data;
		INSERT INTO Result VALUES (recent_data,sum);
		SET i = i+1;

	END WHILE;

END $$

DELIMITER ;

CALL moving_sum('data','x',3,1);
SELECT * FROM Result;

DROP TABLE Result;
DROP PROCEDURE moving_sum;