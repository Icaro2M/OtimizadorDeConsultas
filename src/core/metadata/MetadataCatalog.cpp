#include "MetadataCatalog.h"

#include <algorithm>
#include <stdexcept>

#include "../../utils/StringUtils.h"

MetadataCatalog::MetadataCatalog()
{
    m_tables["endereco"] = TableMetadata{
        "endereco",
        {
            {"idendereco", ColumnType::Integer},
            {"enderecopadrao", ColumnType::Integer},
            {"logradouro", ColumnType::String},
            {"numero", ColumnType::String},
            {"complemento", ColumnType::String},
            {"bairro", ColumnType::String},
            {"cidade", ColumnType::String},
            {"uf", ColumnType::String},
            {"cep", ColumnType::String},
            {"tipoendereco_idtipoendereco", ColumnType::Integer},
            {"cliente_idcliente", ColumnType::Integer}
        }
    };

    m_tables["cliente"] = TableMetadata{
        "cliente",
        {
            {"idcliente", ColumnType::Integer},
            {"nome", ColumnType::String},
            {"email", ColumnType::String},
            {"nascimento", ColumnType::DateTime},
            {"senha", ColumnType::String},
            {"tipocliente_idtipocliente", ColumnType::Integer},
            {"dataregistro", ColumnType::DateTime}
        }
    };

    m_tables["pedido"] = TableMetadata{
        "pedido",
        {
            {"idpedido", ColumnType::Integer},
            {"status_idstatus", ColumnType::Integer},
            {"datapedido", ColumnType::DateTime},
            {"valortotalpedido", ColumnType::Decimal},
            {"cliente_idcliente", ColumnType::Integer}
        }
    };

    m_tables["produto"] = TableMetadata{
        "produto",
        {
            {"idproduto", ColumnType::Integer},
            {"nome", ColumnType::String},
            {"descricao", ColumnType::String},
            {"preco", ColumnType::Decimal},
            {"quantestoque", ColumnType::Decimal},
            {"categoria_idcategoria", ColumnType::Integer}
        }
    };

    m_tables["tipoendereco"] = TableMetadata{
        "tipoendereco",
        {
            {"idtipoendereco", ColumnType::Integer},
            {"descricao", ColumnType::String}
        }
    };

    m_tables["tipocliente"] = TableMetadata{
        "tipocliente",
        {
            {"idtipocliente", ColumnType::Integer},
            {"descricao", ColumnType::String}
        }
    };

    m_tables["status"] = TableMetadata{
        "status",
        {
            {"idstatus", ColumnType::Integer},
            {"descricao", ColumnType::String}
        }
    };

    m_tables["telefone"] = TableMetadata{
        "telefone",
        {
            {"numero", ColumnType::String},
            {"cliente_idcliente", ColumnType::Integer}
        }
    };

    m_tables["pedido_has_produto"] = TableMetadata{
        "pedido_has_produto",
        {
            {"idpedidoproduto", ColumnType::Integer},
            {"pedido_idpedido", ColumnType::Integer},
            {"produto_idproduto", ColumnType::Integer},
            {"quantidade", ColumnType::Decimal},
            {"precounitario", ColumnType::Decimal}
        }
    };

    m_tables["categoria"] = TableMetadata{
        "categoria",
        {
            {"idcategoria", ColumnType::Integer},
            {"descricao", ColumnType::String}
        }
    };
}

bool MetadataCatalog::tableExists(const std::string& tableName) const
{
    return m_tables.contains(StringUtils::toLowerCopy(tableName));
}

bool MetadataCatalog::columnExists(const std::string& tableName, const std::string& columnName) const
{
    auto tableIt = m_tables.find(StringUtils::toLowerCopy(tableName));

    if (tableIt == m_tables.end())
        return false;

    const auto& columns = tableIt->second.columns;
    std::string normalizedColumn = StringUtils::toLowerCopy(columnName);

    auto columnIt = std::find_if(
        columns.begin(),
        columns.end(),
        [&](const ColumnMetadata& column)
        {
            return column.name == normalizedColumn;
        }
    );

    return columnIt != columns.end();
}

ColumnType MetadataCatalog::getColumnType(const std::string& tableName, const std::string& columnName) const
{
    auto tableIt = m_tables.find(StringUtils::toLowerCopy(tableName));

    if (tableIt == m_tables.end())
    {
        throw std::invalid_argument("a tabela '" + tableName + "' nao existe no catalogo.");
    }

    std::string normalizedColumn = StringUtils::toLowerCopy(columnName);

    const auto& columns = tableIt->second.columns;
    auto columnIt = std::find_if(
        columns.begin(),
        columns.end(),
        [&](const ColumnMetadata& column)
        {
            return column.name == normalizedColumn;
        }
    );

    if (columnIt == columns.end())
    {
        throw std::invalid_argument(
            "a coluna '" + columnName + "' nao existe na tabela '" + tableName + "'."
        );
    }

    return columnIt->type;
}