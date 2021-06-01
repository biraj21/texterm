CREATE DATABASE `test`; --create a new db

CREATE TABLE `user`( --create a table

    id INT(2) NOT NULL PRIMARY KEY AUTO_INCREMENT,
    name VARCHAR(25) NOT NULL,
);

INSERT INTO `users`(`id`,`name`) VALUE (1,"pk");
