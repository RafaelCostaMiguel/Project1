#include <iostream>
#include <iomanip>
#include <string>
#include <mysql.h>
#include <vector>

using namespace std;

void conectarAoBanco(MYSQL*& conn) {
    const char* server = "127.0.0.1";
    const char* user = "root";
    const char* password = "cris11";
    const char* database = "Hosp";

    conn = mysql_init(nullptr);
    if (!conn) {
        std::cerr << "Erro ao inicializar MySQL: " << mysql_error(conn) << std::endl;
        exit(EXIT_FAILURE);
    }

    if (!mysql_real_connect(conn, server, user, password, database, 0, nullptr, 0)) {
        std::cerr << "Erro ao conectar ao MySQL: " << mysql_error(conn) << std::endl;
        mysql_close(conn);
        exit(EXIT_FAILURE);
    }
}

class Medico {
private:
	std::string nome, especialidade;
    int crm;

public:
    Medico(string nome, string especialidade, int crm) {
        this->nome = nome;
        this->especialidade = especialidade;
        this->crm = crm;
    }

    int getCrm() const {
        return crm;
    }

	void dados(string nome, string especialidade, int crm) {
        this->nome = nome;
        this->especialidade = especialidade;
        this->crm = crm;
	}

    void consultarLeito(MYSQL* conn) {
        if (mysql_query(conn, "SELECT l.id, l.status, p.nome FROM Leito l LEFT JOIN Paciente p ON l.id = p.id_leito")) {
            cerr << "Erro ao consultar lotacao: " << mysql_error(conn) << endl;
            return;
        }

        MYSQL_RES* res = mysql_store_result(conn);
        if (!res) {
            cerr << "Erro ao obter resultado: " << mysql_error(conn) << endl;
            return;
        }

        MYSQL_ROW row;
        cout << setw(10) << "Leito" << setw(15) << "Status" << setw(20) << "Paciente" << endl;
        cout << "-------------------------------------------" << endl;

        while ((row = mysql_fetch_row(res))) {
            cout << setw(10) << row[0]
                << setw(15) << row[1]
                << setw(20) << (row[2] ? row[2] : "Nenhum")
                << endl;
        }

        mysql_free_result(res);
    }

    void consultarLeito(MYSQL* conn, int leito_id) {
        // Monta a consulta SQL
        string query = "SELECT L.id, L.status, P.nome AS paciente_nome, "
            "M.nome AS medico_nome, M.especialidade "
            "FROM Leito L "
            "LEFT JOIN Paciente P ON L.cpf_paciente = P.cpf "
            "LEFT JOIN Medico M ON L.crm_medico = M.crm "
            "WHERE L.id = " + to_string(leito_id);

        // Executa a consulta
        if (mysql_query(conn, query.c_str())) {
            cerr << "Erro ao executar consulta: " << mysql_error(conn) << endl;
            return;
        }

        
        MYSQL_RES* resultado = mysql_store_result(conn);
        if (!resultado) {
            cerr << "Erro ao obter resultado: " << mysql_error(conn) << endl;
            return;
        }

        
        MYSQL_ROW linha;
        int num_colunas = mysql_num_fields(resultado);

        
        if ((linha = mysql_fetch_row(resultado)) != nullptr) {
            cout << "Informacoes do Leito (ID: " << linha[0] << ")" << endl;
            cout << "Status: " << linha[1] << endl;

            if (linha[2]) 
                cout << "Paciente: " << linha[2] << endl;
            else
                cout << "Paciente: Nao associado" << endl;

            if (linha[3]) 
                cout << "Medico: " << linha[3] << " (Especialidade: " << linha[4] << ")" << endl;
            else
                cout << "Medico: Nao associado" << endl;
        }
        else {
            cout << "Leito com ID " << leito_id << " nao encontrado." << endl;
        }

        
        mysql_free_result(resultado);

    }
};

class Paciente {
private:
    string nome;
    char genero;
    int idade, leito_id, cpf;

public:
    Paciente(string nome, char genero, int idade, int leito_id, int cpf) {
        this->nome = nome;
        this->genero = genero;
        this->idade = idade;
        this->leito_id = leito_id;
        this->cpf = cpf;
    }
};



void cadastrarLeito(MYSQL* conn) {
    int numero_leito;
    std::cout << "Digite o numero do leito: ";
    std::cin >> numero_leito;

    std::string query = "INSERT INTO Leito (id) VALUES (" + std::to_string(numero_leito) + ")";
    if (mysql_query(conn, query.c_str())) {
        std::cerr << "Erro ao cadastrar leito: " << mysql_error(conn) << std::endl;
    }
    else {
        std::cout << "Leito cadastrado com sucesso!" << std::endl;
    }
}

void cadastrarPaciente(MYSQL* conn) {
    std::string nome;
    char genero;
    int idade, leito_id, cpf;

    std::cout << "Digite o CPF do paciente: ";
    std::cin >> cpf;

    std::cout << "Digite o nome do paciente: ";
    std::cin.ignore();
    std::getline(std::cin, nome);

    std::cout << "Digite o genero do paciente (M/F): ";
    std::cin >> genero;
    while (genero != 'M' && genero != 'F') {
        std::cout << "Gênero inválido. Digite 'M' para Masculino ou 'F' para Feminino: ";
        std::cin >> genero;
    }

    std::cout << "Digite a idade do paciente: ";
    std::cin >> idade;

    std::cout << "Digite o ID do leito: ";
    std::cin >> leito_id;

    std::string query = "INSERT INTO Paciente (cpf, nome, idade, genero, data_admissao, id_leito) VALUES (" +
        std::to_string(cpf) + ", '" + nome + "', " + std::to_string(idade) + ", '" + genero + "', CURDATE(), " +
        std::to_string(leito_id) + ")";


    if (mysql_query(conn, query.c_str())) {
        std::cerr << "Erro ao cadastrar paciente: " << mysql_error(conn) << std::endl;
    }
    else {
        std::string update_leito = "UPDATE Leito SET status = 'ocupado' WHERE id = " + std::to_string(leito_id);
        if (mysql_query(conn, update_leito.c_str())) {
            std::cerr << "Erro ao atualizar status do leito: " << mysql_error(conn) << std::endl;
         }
        else {
        	std::string update_leito = "UPDATE Leito SET cpf_paciente = " + std::to_string(cpf) + "  WHERE id = " + std::to_string(leito_id);
            if (mysql_query(conn, update_leito.c_str())) {
                std::cerr << "Erro ao atualizar status do leito: " << mysql_error(conn) << std::endl;
            }
            else {
                std::cout << "Paciente cadastrado e leito atualizado!" << std::endl;
            }
        }
    }
}


void cadastrarMedico(MYSQL* conn, string nome, string especialidade, int crm) {
      
    int leito_id;

    std::cout << "Digite o ID do leito: ";
    std::cin >> leito_id;

    std::string query = "INSERT INTO Medico (nome, crm, especialidade, id_leito) VALUES ('" +
        nome + "', " + std::to_string(crm) + ", '" + especialidade + "', " + std::to_string(leito_id) + ")";

    if (mysql_query(conn, query.c_str())) {
        std::cerr << "Erro ao cadastrar medico: " << mysql_error(conn) << std::endl;
    }
    else {
        std::string update_leito = "UPDATE Leito SET crm_medico = " + std::to_string(crm) + "  WHERE id = " + std::to_string(leito_id);
        if (mysql_query(conn, update_leito.c_str())) {
            std::cerr << "Erro ao atualizar status do leito: " << mysql_error(conn) << std::endl;
        }
        else {
            std::cout << "Medico cadastrado e leito atualizado!" << std::endl;
        }

    }
}


void liberarLeito(MYSQL* conn) {
    int leito_id;
    std::cout << "Digite o ID do leito a liberar: ";
    std::cin >> leito_id;

    std::string query = "UPDATE Leito SET status = 'disponivel' WHERE id = " + std::to_string(leito_id);
    if (mysql_query(conn, query.c_str())) {
        std::cerr << "Erro ao liberar leito: " << mysql_error(conn) << std::endl;
        return;
    }

    query = "DELETE FROM Paciente WHERE id = " + std::to_string(leito_id);
    if (mysql_query(conn, query.c_str())) {
        std::cerr << "Erro ao remover paciente: " << mysql_error(conn) << std::endl;
    }
    else {
        std::cout << "Leito liberado com sucesso!" << std::endl;
	}
}
 
int main() {
	string nome, especialidade;
	int leito_id, crm, crmBusca;
     MYSQL* conn;
    conectarAoBanco(conn);
        
    int opcao;
    bool encontrado;
    std::vector<Medico> medicos;

    do {
        std::cout << "\n--- Gerenciamento de Leitos Hospitalares ---\n";
        std::cout << "1. Cadastrar Leito\n";
        std::cout << "2. Cadastrar Paciente\n";
        std::cout << "3. Consultar Lotacao\n";
        std::cout << "4. Liberar Leito\n";
        std::cout << "5. Cadastrar Medico\n";
        std::cout << "6. Listar todos os Leitos\n";
        std::cout << "0. Sair\n";
        std::cout << "Escolha uma opcao: ";
        std::cin >> opcao;

        switch(opcao) {
        case 1:
            cadastrarLeito(conn);
            break;
        case 2:
            cadastrarPaciente(conn);
            break;
        case 3:
            std::cout << "Digite o ID do leito: ";
            std::cin.ignore();
            std::cin >> leito_id;

            std::cout << "Função exclusiva para Medicos ## Digite seu CRM para autenticacao: ";
            std::cin.ignore();
            cin >> crmBusca;

            encontrado = false;
            for (auto& medico : medicos) {
                if (medico.getCrm() == crmBusca) {
                    medico.consultarLeito(conn, leito_id);
                    encontrado = true;
                    break;
                }
            }
            if (!encontrado) {
                std::cout << "Médico não encontrado!" << std::endl;
            }
            break;
        case 4:
            liberarLeito(conn);
             break;
        case 5:
            std::cout << "Digite o nome do profissional: ";
            std::cin.ignore();
            std::getline(std::cin, nome);

            cout << "Digite a especialidade do medico: ";
            getline(cin, especialidade);

            cout << "Digite o CRM do medico: ";
            cin >> crm;

            cadastrarMedico(conn, nome, especialidade, crm);
            medicos.emplace_back(nome, especialidade, crm);
            break;
        case 6:  
            std::cout << "Função exclusiva para Medicos ## Digite seu CRM para autenticacao: ";
            std::cin.ignore();
            cin >> crmBusca;

            encontrado = false;
            for (auto& medico : medicos) {
                if (medico.getCrm() == crmBusca) {
                    medico.consultarLeito(conn);
                    encontrado = true;
                	break;
                }
            }
            if (!encontrado) {
                std::cout << "Médico não encontrado!" << std::endl;
            }
            break;
        case 0:
            std::cout << "Encerrando o sistema..." << std::endl;
            break;
        default:
            std::cout << "Opcao invalida, tente novamente." << std::endl;
        }
    } while (opcao != 0);

    mysql_close(conn);
    return 0;
}
