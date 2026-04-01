#include "MetadataCatalog.h"

#include <algorithm>
#include "../utils/StringUtils.h"

MetadataCatalog::MetadataCatalog()
{
    m_tables["endereco"] = TableMetadata{ "endereco", {"idendereco", "enderecopadrao", "logradouro", "numero", "complemento", "bairro", "cidade", "uf", "cep", "tipoendereco_idtipoendereco", "cliente_idcliente"} };
    m_tables["cliente"] = TableMetadata{ "cliente", {"idcliente", "nome", "email", "nascimento", "senha", "tipocliente_idtipocliente", "dataregistro"} };
    m_tables["pedido"] = TableMetadata{ "pedido", {"idpedido", "status_idstatus", "datapedido", "valortotalpedido", "cliente_idcliente"} };
    m_tables["produto"] = TableMetadata{ "produto", {"idproduto", "nome", "descricao", "preco", "quantestoque", "categoria_idcategoria"} };
    m_tables["tipoendereco"] = TableMetadata{ "tipoendereco", {"idtipoendereco", "descricao"} };
    m_tables["tipocliente"] = TableMetadata{ "tipocliente", {"idtipocliente", "descricao"} };
    m_tables["status"] = TableMetadata{ "status", {"idstatus", "descricao"} };
    m_tables["telefone"] = TableMetadata{ "telefone", {"numero", "cliente_idcliente"} };
    m_tables["pedido_has_produto"] = TableMetadata{ "pedido_has_produto", {"idpedidoproduto", "pedido_idpedido", "produto_idproduto", "quantidade", "precounitario"} };
    m_tables["categoria"] = TableMetadata{ "categoria", {"idcategoria", "descricao"} };
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

    return std::find(columns.begin(), columns.end(), normalizedColumn) != columns.end();
}