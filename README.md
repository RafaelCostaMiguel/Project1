# Project1

#ESTRUTURA MySQL

CREATE DATABASE Hosp;
USE Hosp;
CREATE TABLE Paciente (
    cpf INT AUTO_INCREMENT PRIMARY KEY,
    nome VARCHAR(100) NOT NULL,
    idade INT NOT NULL,
    genero ENUM('M', 'F') NOT NULL,
    data_admissao DATE NOT NULL,
    id_leito INT
);
CREATE TABLE Leito (
    id INT AUTO_INCREMENT PRIMARY KEY,
    status ENUM('Disponível', 'Ocupado', 'Manutenção') NOT NULL,
    cpf_paciente INT,
    crm_medico INT,
	FOREIGN KEY (crm_medico) REFERENCES Medico(crm),
    FOREIGN KEY (cpf_paciente) REFERENCES Paciente(cpf)
);
CREATE TABLE Medico (
    crm INT AUTO_INCREMENT PRIMARY KEY,
    nome VARCHAR(100) NOT NULL,
    id_leito INT,
    especialidade VARCHAR(50)
);
