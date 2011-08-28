module RevisionTags

  GLOBAL_DATE_FORMAT = '%Y-%m-%dT%I:%M:%S%z'

  class Block < Liquid::Block

    def initialize(tag_name, text, tokens)
      super
      @text = text
    end

    def render(context)
      content = super
      @date = RevisionTags.parse_date_with_timezone(@text, "EST")
      tag = case self.class.name
            when /Inserted/
              "ins"
            when /Deleted/
              "del"
            end
      open_tag = %{<#{tag} datetime="#{RevisionTags.html_global_date(@date)}">}
      %{#{open_tag}#{content.first}</#{tag}>}
    end

  end

  class InsertedTag < Block

  end

  class DeletedTag < Block

  end

  def self.html_global_date(date)
    formatted_date = date.strftime GLOBAL_DATE_FORMAT
    formatted_date.gsub!(/([-+]\d\d)(\d\d)/, '\1:\2')
    return formatted_date
  end

  def self.parse_date_with_timezone(date, tz="UTC")
    date << tz unless date =~ /#{tz}\z/
    DateTime.parse(date)
  end

  module GlobalDateFilter

    def html_global_date(date)
      RevisionTags.html_global_date(date)
    end

  end

end

Liquid::Template.register_tag 'inserted', RevisionTags::InsertedTag
Liquid::Template.register_tag 'deleted', RevisionTags::DeletedTag

Liquid::Template.register_filter(RevisionTags::GlobalDateFilter)
